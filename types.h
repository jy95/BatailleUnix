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

#if ! defined (_TYPES_JOUEUR_H_)
#define _TYPES_JOUEUR_H_

//message

#define INSCRIPTION 1
#define INSCRIPTION_REUSSIE 2
#define INSCRIPTION_RATEE 3
#define PARTIE_COMMENCEE 4
#define DISTRIBUTION_CARTE 5
#define DEMANDE_CARTE 6
#define JOUE_CARTE 7
#define PLUS_DE_CARTE 8
#define MANCHE_TERMINEE 9
#define UPDATE_POINTS 10
#define FIN_PARTIE 11
#define TIMEOUT_JOUEUR 12
#define NO_MORE_JOUEURS 13
#define NO_WANT_TO_PLAY 14
#define ACCEPTED_DECONNEXION 15
#define GAGNE_CARTE 16
#define PARTIE_ANNULEE 17

#define CLE_PUBLIQUE 4747
#define NOMBRE_MAX 3
#define NOMBRE_CARTES 52
#define TIMER 10


#define TRUE   1
#define FALSE  0
#define PORT 18197
#define NOMBRE_MANCHE 5

#define SYS(call) ((call) == -1) ? (perror(#call ": ERROR"),exit(1)) : 0

typedef struct Carte {
	int valeur;
	enum Couleur {COEUR,CARREAU,TREFLE,PIQUE} couleur;
} Carte;

typedef struct Message {
	int type;
	char message[256];
	Carte cartes[NOMBRE_MAX];
} Message;

typedef struct ScoresJoueurs {
	char nomJoueurs[NOMBRE_MAX][50];
	int  scores[NOMBRE_MAX];
	int taille;
} Scores;

typedef struct Joueur {
	char nom[50];
	int points;
	int nombreCarteDeck;
	int nombreCarteReserve;
	Carte deck[52];
	Carte reserve[52];
	int indice;
} Joueur;

typedef struct Noeud {
	Carte* carte;
	struct Noeud* suivant;
} Noeud;

#endif
