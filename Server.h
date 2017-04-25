
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

#if ! defined (_SERVEUR_H_)
#define _SERVEUR_H_

void partie();

void manche();

void init(int max_clients,int (* client_socket)[],int * master_socket,struct sockaddr_in * address , int * addrlen ,int opt);

void writeMessageForClient(int index, Message message[],fd_set * readfds, int (* client_socket)[] , int max_Client); 

void readMessage(int index, fd_set * readfds, Message (* messages)[] , int (* client_socket)[] , int max_clients);

int acceptConnexion(fd_set * readfds, int * master_socket, int *nombreCourant     , int addrlen , int max_clients , int (* client_socket)[] , struct sockaddr_in * address );

Message inscription(Message message, int * nombreCourant , Joueur(* joueurs)[], int indice);

void distributionCarte( Joueur(* joueurs)[] , int nombreCourant, int (* client_socket)[], Message (* messages)[], fd_set * readfds );

void tour(int activity,int indice, Message message, int* nombreCartesJouees, Carte* cartesJouees);

void checkCarteGagnante(Carte* carte);

#endif
