

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

#if ! defined (_CLIENT_H_)
#define _CLIENT_H_

void messageServeurPourClient(Message a);
void afficherReponse(char * message);
Message receiveResponse(int sockfd);
Message readTask();
Message inscription();
void sendRequest(Message m, int sockfd);
void quitter(int sockfd);
void recupererCarte(Message * message , Joueur * joueur);
void choixCarte(Message* message);
void afficherDeck();
void calculerScore(Message* message);
int recuUnMessage(int sockfd , fd_set * readfds, struct timeval * tv);

#endif
