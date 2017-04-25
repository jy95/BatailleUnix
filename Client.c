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
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#include "types.h"
#include "Client.h"
#include "messages.h"
#include "AfficherCarte.h"
#include "MemoirePartagee.h"

int estInscrit = 0;
int joue = 0;
struct sigaction sig;
int sockfd;
Joueur joueur;
int score = 0;


void sig_handler(int signo){

	if (signo == SIGINT || signo == SIGQUIT) {
		quitter(sockfd);
	}
}


int main(int argc, char *argv[]) {
	int portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	sig.sa_handler = sig_handler;
	srand(time(NULL));
	sigaction(SIGQUIT,&sig,NULL);
	sigaction(SIGINT,&sig,NULL);

	char buffer[256];
	Joueur joueur;
	joueur.nombreCarteDeck = 0;
	joueur.nombreCarteReserve = 0;
	int recuQuelqueChose;
	fd_set readfds;

	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}

	portno = atoi(argv[2]);

	/* Create a socket point */
	SYS(sockfd = socket(AF_INET, SOCK_STREAM, 0));

	server = gethostbyname(argv[1]);

	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);


	Message envoi = inscription();
	struct timeval tv = {TIMER, 0};

	/* Now connect to the server */
	SYS(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)));

	sendRequest(envoi,sockfd);

	while(1) {
		FD_ZERO(&readfds);
		FD_SET(sockfd,&readfds);
		recuQuelqueChose = recuUnMessage(sockfd , &readfds, &tv); 

		if (recuQuelqueChose == 1) {
			Message recu;
			recu = receiveResponse(sockfd);

			messageServeurPourClient(recu);


			Message messageAEnvoyer;
			messageAEnvoyer.type = 0; // éviter envoi inutile
			messageAEnvoyer = readTask(recu);


			if (messageAEnvoyer.type != 0) {

				sendRequest(messageAEnvoyer, sockfd);
			}

		}
	}
	return 0;

}

void quitter(int sockfd) {
	char msgAtransformer[255];

	Message envoi;
	envoi.type = NO_WANT_TO_PLAY;
	strncpy(envoi.message,"\0",1);

	/*Envoi du message*/
	sendRequest(envoi,sockfd);

	Message recu = receiveResponse(sockfd);
	messageServeurPourClient(recu);


	SYS(close(sockfd));
	exit(1);
}

void messageServeurPourClient(Message a) {
	char communication[255];

	switch(a.type) {
		case INSCRIPTION_REUSSIE :
			strncpy(communication,"Inscription Confirmée\n",255);
			break;

		case INSCRIPTION_RATEE :
			strncpy(communication,"Echec de l'inscription\n",255);
			break;

		case PARTIE_COMMENCEE :
			//strncpy(communication,"Debut de partie\n",255);
			break;

		case ACCEPTED_DECONNEXION :
			strncpy(communication,"Deconnexion avec succès\n",255);
			break;

		case DISTRIBUTION_CARTE :
			strncpy(communication,"Distribution des cartes en cours\n",255);
			break;

		default:
			break;
	}
	afficherReponse(communication);

}

Message inscription() {

	char message[255];
	char buffer[255];
	char msgAtransformer[255];
	Message envoi;
	int taille;

	strncpy(message,"Veuillez choisir un pseudo :\n",255);
	printf("%s",message);


	if ((taille = read(0,buffer,255)) > 0) {
		strcpy(joueur.nom, buffer);
		/*Construire le message*/
		envoi.type = INSCRIPTION;
		buffer[taille-1] = '\0';
		strncpy(envoi.message,buffer,taille);

		return envoi;
	} else {
		exit(1);
	}
}


void sendRequest(Message m, int sockfd) {

	/* Send message to the server */
	SYS(send(sockfd, &m , sizeof(Message),0)); 

}

Message receiveResponse(int sockfd) {

	Message mes;
	SYS(recv(sockfd, &mes, sizeof(Message),0));
	return mes;
}

void afficherReponse(char * s){
	printf("%s \n",s);
}

Message readTask(Message messageRecu) {

	Message message;
	message.type = 0;
	Carte choix;

	switch (messageRecu.type){
		case DEMANDE_CARTE :
			choixCarte(&message);
			break;

		case FIN_PARTIE:
			printf("Partie terminee\n");
			exit(1);
			break;

		case MANCHE_TERMINEE:
			printf("manche terminee\n");
			memset(joueur.deck,0,sizeof(joueur.deck));
			memset(joueur.reserve,0,sizeof(joueur.reserve));
			joueur.nombreCarteDeck = 0;
			joueur.nombreCarteReserve = 0;
			calculerScore(&message);

			break;

		case GAGNE_CARTE:
			recupererCarte(&messageRecu,&joueur);
			break;

		case DISTRIBUTION_CARTE:
			recupererCarte(&messageRecu,&joueur);
			break;

		case PARTIE_ANNULEE:
			quitter(sockfd);
			break;

		default :

			break;

	}

	return message;
}


void calculerScore(Message* message){
	int scoreTotal = 0;
	int i;
	for(i = 0; i < joueur.nombreCarteDeck; i++){
		scoreTotal += joueur.deck[i].valeur;
	}
	for(i = 0; i < joueur.nombreCarteReserve; i++){
		scoreTotal += joueur.reserve[i].valeur;
	}
	message->type = UPDATE_POINTS;
	strcpy(message->message, "score fin de manche");
	Carte carte;
	carte.valeur = scoreTotal;
	carte.couleur = 0;
	message->cartes[0] = carte;
}

void recupererCarte(Message * message , Joueur * joueur) {
	int i;
	int * nbreCarte = &(joueur->nombreCarteReserve);
	Carte* deckUtilise = joueur->reserve;
	int nombreCartes = NOMBRE_MAX;
	if(message->type == DISTRIBUTION_CARTE){
		nbreCarte = &(joueur->nombreCarteDeck);
		deckUtilise = joueur->deck;
		nombreCartes = 1;
	}
	// récupérer 4 cartes au maximum
	for (i = 0 ; i < nombreCartes ; i++) {

		Carte aAjouter = (message->cartes)[i];

		if (aAjouter.valeur > 0 && aAjouter.valeur <= 13) {

			deckUtilise[(*nbreCarte)++] = aAjouter;

		}

	}

}
void afficherDeck(){
	int i;
	for(i = 0; i < joueur.nombreCarteDeck; i++){
		printf("%d) ", i);
		afficherCarte(&joueur.deck[i]);
	}
}

void choixCarte(Message* message){
	Carte carte;
	char buffer[3];
	int i;

	if(joueur.nombreCarteDeck == 0){
		printf("deck vide\n");
		if(joueur.nombreCarteReserve != 0){
			Carte* deck = joueur.deck;
			for(i = 0; i < joueur.nombreCarteReserve; i++){
				joueur.deck[i] = joueur.reserve[i];
			}
			joueur.nombreCarteDeck = joueur.nombreCarteReserve;
			joueur.nombreCarteReserve = 0;
		} else {
			// le joueur n'a plus de cartes -> notifier serveur
			// renvoit d'une struct carte vide 
			printf("reserve vide\n");
			message->type = PLUS_DE_CARTE;
			return;
		}
	}

	
	printf("Veuillez choisir une carte:\n");
	//printf("nombrecarteDeck = %d\n", joueur.nombreCarteDeck);
	//printf("nombrecarteReserve = %d\n", joueur.nombreCarteReserve);
	afficherDeck();

	int indice;
	if(read(0,buffer,3) > 0){
		indice = atoi(buffer);
	}
	else{
		quitter(sockfd);
	}
	printf("indice = %d\n", indice);

	while(indice < 0 || indice >= joueur.nombreCarteDeck){
		printf("Veuillez entrez un nombre Correct\n");
		if(read(0,buffer,3) > 0){
			indice = atoi(buffer);
		}
	}

	// prend la carte
	// switcher position carte utilisée - dernier carte
	carte = joueur.deck[indice];	
	joueur.deck[indice] = joueur.deck[--joueur.nombreCarteDeck];
	message->cartes[0] = carte;
	message->type = JOUE_CARTE;
	strcpy(message->message, "jouer carte");
}

int recuUnMessage(int sockfd , fd_set * readfds, struct timeval * tv) {
	int activity;
	SYS(activity = select( sockfd + 1 , readfds , NULL , NULL , tv));
	if (activity == 0) {
		// timer expiré
		return -1;
	} else {
		return 1;
	}

}
