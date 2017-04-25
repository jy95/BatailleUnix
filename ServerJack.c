/*
#############################################################
# Projet Unix - Bataille
# GROUPE VERWILGHEN - YAKOUB
#
# Etudiant 1 : VERWILGHEM Maxime
# Etudiant 2 : YAKOUB Jacques
# SERIE 1
############################################################
 */

#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <signal.h>
#include <time.h> 
#include "types.h"
#include "Server.h"
#include "messages.h"
#include "MemoirePartagee.h"


int debutPartie = 0;
int nombreCourant = 0;
Message messagesRecus[NOMBRE_MAX];
Message messagesAEnvoyer[NOMBRE_MAX];
struct sigaction sig;
int nombreMancheMax = NOMBRE_MANCHE;
int nombreMancheEffectues = 0;

int master_socket , addrlen , new_socket , client_socket[NOMBRE_MAX] , max_clients = NOMBRE_MAX , activity, i , valread , sd;
int tourEnCours;
int absents[NOMBRE_MAX];

//set of socket descriptors
fd_set readfds;

void sig_handler(int signo){
	if ( signo == SIGINT) {
		supprimerSegment();
	}
}


int main(int argc , char *argv[])
{

	int j;
	for(j = 0; j < NOMBRE_MAX; j++){
		absents[j] = 1;
	}
	int opt = TRUE;
	int max_sd;
	int indice;
	struct timeval timer = {TIMER,0};
	struct sockaddr_in address;
	char buffer[255];
	Joueur joueurs[4];
	Message aTraiter;
	int scoreJoueur;
	int nombreCartesJouees = 0;
	Carte cartesJouees[NOMBRE_MAX];

	if (argc == 2) {
		nombreMancheMax = atoi(argv[1]);
		printf("%d \n", nombreMancheMax);
	}

	//initialise all client_socket[] to 0 so not checked
	init(max_clients,&client_socket,&master_socket,&address,&addrlen,opt);


	// memoire Partagée
	SYS(creerSegment());

	while(TRUE) 
	{
		//clear the socket set
		FD_ZERO(&readfds);

		//add master socket to set
		FD_SET(master_socket, &readfds);

		printf("attente d'un mesage\n");
		int activity = acceptConnexion(&readfds, &master_socket , &nombreCourant, addrlen, NOMBRE_MAX , &client_socket, &address);
		if (activity != -1 ) {
			printf("recu un message\n");

			for ( indice = 0 ; indice < NOMBRE_MAX ; indice++) {

				// si dans l'ensemble de fd
				if ( FD_ISSET(client_socket[indice], &readfds) && client_socket[indice] != master_socket) {

					// lire le message 
					readMessage(indice, &readfds, &messagesRecus , &client_socket , NOMBRE_MAX);				
					// action en fonction du message
					switch(messagesRecus[indice].type) {
						case INSCRIPTION:
							aTraiter = inscription(messagesRecus[indice], &nombreCourant , &joueurs ,indice);
							messagesAEnvoyer[indice] = aTraiter;
							writeMessageForClient(indice,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);
							break;

						case NO_WANT_TO_PLAY:
							printf("NO_WANT_TO_PLAY\n");
							printf("nombreCourant = %d\n",nombreCourant);
							aTraiter.type = ACCEPTED_DECONNEXION;
							strncpy(aTraiter.message,"\0",1);
							messagesAEnvoyer[indice] = aTraiter;
							writeMessageForClient(indice,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);

							FD_CLR(client_socket[indice],&readfds);
							client_socket[indice] = 0;
							Carte vide;
							vide.valeur = 0;
							vide.couleur = 0;
							cartesJouees[indice] = vide;
							nombreCourant--;
							if (debutPartie == 1 && nombreCourant == 1) {
								aTraiter.type = FIN_PARTIE;
								strcpy(aTraiter.message, "Tu as gagne\0");
								for (j = 0 ; j < NOMBRE_MAX ; j++) {

									if (client_socket[j] != 0) {
										messagesAEnvoyer[j] = aTraiter;
										FD_SET(client_socket[j],&readfds);
										writeMessageForClient(j,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);
										debutPartie = 0;
										tourEnCours = 0;
									}
								}
							}
							break;

						case UPDATE_POINTS:
							scoreJoueur = atoi(messagesRecus[indice].message);
							ecrireMemoire(scoreJoueur , indice , joueurs[indice].nom);
							break;

						case PLUS_DE_CARTE:
							// fin de la manche courrante

							// demande scores
							for (j = 0 ; j < NOMBRE_MAX ; j++) {
								aTraiter.type = MANCHE_TERMINEE;
								messagesAEnvoyer[j] = aTraiter;

								if (client_socket[j] != 0) {
									FD_SET(client_socket[j],&readfds);
									writeMessageForClient(j,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);
								}

							}

							if (nombreMancheEffectues++ == nombreMancheMax) {
								// FIN DE LA PARTIE

								for (j = 0 ; j < NOMBRE_MAX ; j++) {
									aTraiter.type = FIN_PARTIE;
									messagesAEnvoyer[j] = aTraiter;

									if (client_socket[j] != 0) {
										FD_SET(client_socket[j],&readfds);
										writeMessageForClient(j,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);
									}

								}

							} else {
								// LANCEMENT d'une nouvelle manche

								// organiser ici distributions des cartes pour les joueurs
								distributionCarte(&joueurs,nombreCourant,&client_socket , &messagesAEnvoyer , &readfds);
								printf("PARTIE COMMENCEE \n");
								aTraiter.type = PARTIE_COMMENCEE;
								strncpy(aTraiter.message,"\0",1);


								for (indice = 0 ; indice < NOMBRE_MAX ; indice++) {
									printf("envoi message client %d\n", indice);
									messagesAEnvoyer[indice] = aTraiter;
									writeMessageForClient(indice,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);
								}


								printf("DEMARRAGE MANCHE X\n");
								nombreMancheEffectues++;
								aTraiter.type = DEMANDE_CARTE;
								strncpy(aTraiter.message,"\0",1);

								for (indice = 0 ; indice < NOMBRE_MAX ; indice++) {
									messagesAEnvoyer[indice] = aTraiter;
									writeMessageForClient(indice,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);
								}
								printf("tour->X\n");
								tourEnCours = 1;
							}


							break;
						case JOUE_CARTE:
							if(tourEnCours == 1 && debutPartie == 1){
								tour(activity, indice, messagesRecus[indice], &nombreCartesJouees, cartesJouees);
							}
							break;

						default:
							// Ne rien faire
							break;
					}

				}
			}

		} else {
			printf("timer expire\n");
			printf("debutPartie = %d\n", debutPartie);
			printf("tourEnCours = %d\n", tourEnCours);

			// personne ne vient après le premier joueur par exemple
			if (nombreCourant == 1 && debutPartie == 0 ) {
				printf("PARTIE ANNULEE \n");
				Message mes;
				mes.type = PARTIE_ANNULEE;
				for (indice = 0 ; indice < NOMBRE_MAX ; indice++) {
					if(client_socket[indice] != 0){
						FD_SET(client_socket[indice], &readfds);
						messagesAEnvoyer[indice] = mes;
						writeMessageForClient(indice,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);
					}
				}
			}

			// cas du timer du select expiré
			else if (nombreCourant >= 2 && debutPartie == 0 ) {
				printf("Nombre de joueurs %d \n",nombreCourant);
				debutPartie = 1;

				// organiser ici distributions des cartes pour les joueurs
				distributionCarte(&joueurs,nombreCourant,&client_socket , &messagesAEnvoyer , &readfds);
				printf("PARTIE COMMENCEE \n");
				aTraiter.type = PARTIE_COMMENCEE;
				strncpy(aTraiter.message,"\0",1);


				for (indice = 0 ; indice < NOMBRE_MAX ; indice++) {
					printf("envoi message client %d\n", indice);
					messagesAEnvoyer[indice] = aTraiter;
					writeMessageForClient(indice,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);
					if (FD_ISSET(client_socket[indice],&readfds)) {
						printf("avant meemoire\n");
						ecrireMemoire(0 , indice , joueurs[indice].nom);
						printf("apres meemoire\n");
					}
				}


				printf("DEMARRAGE MANCHE 1\n");
				nombreMancheEffectues++;
				aTraiter.type = DEMANDE_CARTE;
				strncpy(aTraiter.message,"\0",1);

				for (indice = 0 ; indice < NOMBRE_MAX ; indice++) {
					messagesAEnvoyer[indice] = aTraiter;
					writeMessageForClient(indice,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);
				}
				printf("tour->1\n");
				tourEnCours = 1;

			}

			// cas d'une fin du timer de jeu 
			else if ( debutPartie == 1 && tourEnCours == 1) {
				printf("est-ce que je pase ici?\n");

				Message bidon;
				tour(activity, 0, bidon, &nombreCartesJouees, cartesJouees);
			}

		}
	} 
	supprimerSegment();
	return 0;
}

void tour(int activity, int indice, Message message, int* nombreCartesJouees, Carte* cartesJouees){
	printf("activity = %d\n", activity);
	printf("indice = %d\n", indice);
	printf("*nombreCartesJouees = %d\n", *nombreCartesJouees);

	if(activity < 0){
		if(*nombreCartesJouees < 2){


			printf("je rentre ici\n");
			// terminer le tour
			tourEnCours = 0;

			// envoyer message
			Message mes;
			mes.type = FIN_PARTIE;

			// parcourir tableau 
			int i;
			for (i = 0 ; i < NOMBRE_MAX ; i++) {
				printf("client_socket[%d] = %d\n", i, client_socket[i]);
				printf("absents[%d] = %d\n", i, absents[i]);
				if (client_socket[i] != 0) {
					FD_SET(client_socket[i], &readfds);
					if(absents[i] == 0){
						strcpy(mes.message,"TU AS GAGNE \0");
						printf("gagne\n");
					}else{
						strcpy(mes.message,"deconnecte\0");
						printf("perdu\n");
					}
					messagesAEnvoyer[i] = mes;
					printf("envoi message au gagnant car autre deco = %d\n", i);
					printf("message envoyer = %s\n", messagesAEnvoyer[i].message);
					writeMessageForClient(i,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);
				}
			}

			// avertir joueur 


		}
	}else if(activity == 1){
		Carte carte = message.cartes[0];
		if(carte.valeur > 0 && carte.valeur <= 13){
			cartesJouees[indice] = carte;
			absents[indice] = 0;
			(*nombreCartesJouees)++;
		}
		if(*nombreCartesJouees == nombreCourant){
			*nombreCartesJouees = 0;
			checkCarteGagnante(cartesJouees);
		}
	}
}

void checkCarteGagnante(Carte* cartesJouees){
	int i;
	int gagnant = 0;
	Message message;

	// il pourrait avoir des troues dans le tableau
	// voici une manière de le faire
	for(i = 0; i < NOMBRE_MAX; i++){
		if(cartesJouees[i].valeur > cartesJouees[gagnant].valeur){
			if(cartesJouees[gagnant].valeur != 1)
				gagnant = i;
		}
		else if(cartesJouees[i].valeur == cartesJouees[gagnant].valeur){
			if(cartesJouees[i].couleur > cartesJouees[gagnant].couleur){
				gagnant = i;
			}	
		}
		else if(cartesJouees[i].valeur == 1){
			gagnant = i;
		}
		message.cartes[i] = cartesJouees[i];
	}
	message.type = GAGNE_CARTE;
	strcpy(message.message, "cartes gagnees\0");

	//envoyer message a indice du gagnant
	messagesAEnvoyer[gagnant] = message;
	FD_SET(client_socket[gagnant],&readfds);
	writeMessageForClient(gagnant,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);
	printf("envoi message au gagnant = %d\n", gagnant);

	// vider la mémoire prise par les anciennes cartes
	memset(cartesJouees,0,sizeof(cartesJouees));

	message.type = DEMANDE_CARTE;
	strcpy(message.message, "demande une carte\0");
	for(i = 0; i < NOMBRE_MAX; i++){
		if(client_socket[i] != 0){
			messagesAEnvoyer[i] = message;
			FD_SET(client_socket[i],&readfds);
			writeMessageForClient(i,messagesAEnvoyer, &readfds, &client_socket, NOMBRE_MAX);
		}
	}
	printf("demande nouvelles cartes\n");
}

Message inscription(Message message, int * nombreCourant, Joueur(* joueurs)[] , int socket_indice ){
	printf("INSCRIPTION\n");
	Message retour;
	if(*nombreCourant > NOMBRE_MAX || debutPartie == 1){
		retour.type = INSCRIPTION_RATEE;
	} else {
		retour.type = INSCRIPTION_REUSSIE;
		strncpy(((*joueurs)[*nombreCourant]).nom,message.message,strlen(message.message));
		((*joueurs)[*nombreCourant]).indice = socket_indice;
		(*nombreCourant)++;
	}
	//strncpy(((*joueurs)[*nombreCourant]).nom,"\0",1);
	return retour;
}

void init(int max_clients,int (* client_socket)[],int * master_socket,struct sockaddr_in * address , int * addrlen ,int opt) {
	int i;
	for (i = 0; i < max_clients; i++) 
	{
		(*client_socket)[i] = 0;
	}

	//create a master socket
	if( (*master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	//set master socket to allow multiple connections , this is just a good habit, it will work without this
	SYS(setsockopt(*master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)));

	(*address).sin_family = AF_INET;
	((*address).sin_addr).s_addr = INADDR_ANY;
	(*address).sin_port = htons( PORT );


	//bind the socket to localhost port 8888
	SYS(bind(*master_socket, (struct sockaddr *) (address), sizeof(*address)));

	//try to specify maximum of NOMBRE_MAX pending connections for the master socket
	SYS(listen(*master_socket, NOMBRE_MAX));

	//accept the incoming connection
	*addrlen = sizeof(address);
	printf("Waiting for connections ...\n");

}

void readMessage(int index,fd_set * readfds, Message (* messages)[] , int (* client_socket)[] , int max_clients ) {

	// char buffer[255];
	//valread = recv( sd , buffer, 255, 0)) > 0
	//str2msg(buffer,&recu);

	int sd;
	sd = (*client_socket)[index];
	Message message;
	if (FD_ISSET( sd , readfds)) 
	{

		// read the incoming message
		if (recv( sd , &message, sizeof(Message), 0)){
			(*messages)[index] = message;
		} else {
			// deconnexion d'un joueur
			FD_CLR(sd,readfds);
			(*client_socket)[index] = 0;
			close(sd);
			nombreCourant--;
		}
	}
}

int acceptConnexion(fd_set * readfds, int * master_socket, int *nombreCourant , int addrlen , int max_clients , int (* client_socket)[] , struct sockaddr_in * address ) {
	int i;
	int valread;
	char message[1024];
	int new_socket;
	int sd;
	int max_sd;
	struct timeval tv = {TIMER, 0};
	int nombre;
	max_sd = *master_socket;

	//add child sockets to set
	for ( i = 0 ; i < max_clients ; i++) 
	{
		//socket descriptor
		sd = (*client_socket)[i];

		//if valid socket descriptor then add to read list
		if(sd > 0)
			FD_SET( sd , readfds);

		//highest file descriptor number, need it for the select function
		max_sd = (sd > max_sd) ? sd : max_sd;
	}
	//wait for an activity on one of the sockets , timeout
	int activity;
	SYS(activity = select( (max_sd) + 1 , readfds , NULL , NULL , &tv));


	if (activity == 0) {
		// timer expiré
		return -1;
	} else {

		// Nouvelle connexion d'un user
		if (FD_ISSET(*master_socket, readfds)) 
		{
			SYS((new_socket = accept(*master_socket, (struct sockaddr *) address, (socklen_t*) &addrlen)));

			printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa( address->sin_addr) , ntohs( (*address).sin_port));

			// ajouter le nouveau user
			for (nombre = 0 ; nombre < NOMBRE_MAX ; nombre++) {

				if( (*client_socket)[nombre] == 0 )
				{
					(*client_socket)[nombre] = new_socket;
					FD_SET(new_socket,readfds);
					break;
				}

			}
		}

		return 1;
	}
}

void writeMessageForClient(int i , Message message[], fd_set * readfds, int (* client_socket)[]  , int max_client) {
	int sd;
	sd = (*client_socket)[i];

	// check si message valide
	if (FD_ISSET( sd , readfds) )
	{

		Message toSend = message[i];
		send(sd,&toSend,sizeof(Message), 0);

	}

}

void distributionCarte( Joueur(* joueurs)[] , int nbreJoueurs, int (* client_socket)[], Message (* messages)[], fd_set * readfds ) {
	int val;
	int coul;
	Carte cartes[52];
	int indice = 0;
	int joueur = 0;
	int i = 0;
	int j = 0;
	int cartesRestantes = 0;

	for(val = 1; val < 14; val++){
		for(coul = 1; coul < 5; coul++){
			Carte carte;
			carte.valeur = val;
			carte.couleur = coul;
			cartes[indice++] = carte;
			cartesRestantes++;
		}
	}

	Message mes;


	for(i = 0; i<nbreJoueurs; i++){
		FD_SET((*client_socket)[i], readfds);
		int cartesParJoueur = 52/nbreJoueurs;
		for(j = 0; j < cartesParJoueur; j++){
			//printf("joueur = %d, valeur = %d, couleur = %d\n", i+1, cartes[i][j].valeur, cartes[i][j].couleur);

			srand(time(NULL)*i);
			int random = rand()%cartesRestantes;
			Carte carteADonner = cartes[random];
			cartes[random] = cartes[--cartesRestantes];
			mes.cartes[0] = carteADonner;
			mes.type = DISTRIBUTION_CARTE;
			strncpy(mes.message,"\0",1);
			(*messages)[(*joueurs)[i].indice] = mes;
			writeMessageForClient((*joueurs)[i].indice, *messages, readfds, client_socket , nbreJoueurs);
		}
	}

	printf("DISTRIBUTION_CARTE FINI \n");
}
