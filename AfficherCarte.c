// sequences "d'escape caracters" pour afficher les codes des cartes.
// mai 2016

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "AfficherCarte.h"

#define PIQUE  "\xE2\x99\xA0"
#define TREFLE "\xE2\x99\xA3"
#define COEUR  "\xE2\x99\xA5"
#define CARREAU  "\xE2\x99\xA6"

// un peu de couleur

#define noirTexte 30
#define rougeTexte 31
#define vertTexte 32
#define jauneTexte 33
#define bleuTexte 34
#define magentaTexte 35
#define cyanTexte 36
#define blancTexte 37

#define noirFond 40
#define rougeFond 41
#define vertFond 42
#define jauneFond 43
#define bleuFond 44
#define magentaFond 45
#define cyanFond 46
#define blancFond 47


#define couleurOn(A,B,C) printf("\033[%d;%d;%dm",A,B,C)
#define couleurOff() printf("\033[0m");

void afficherCarte(Carte *carte){
	char* toPrint;
	switch(carte->couleur){
		case 1:
			couleurOn(0,rougeTexte,blancFond);
			printf(COEUR);
			printf("%d", carte->valeur);
			couleurOff();
		break;
		case 2:
			couleurOn(0,rougeTexte,blancFond);
			printf(CARREAU);
			printf("%d", carte->valeur);
			couleurOff();
		break;
		case 3:
			couleurOn(0,noirTexte,blancFond);
			printf(TREFLE);
			printf("%d", carte->valeur);
			couleurOff();
		break;
		case 4:
			couleurOn(0,noirTexte,blancFond);
			printf(PIQUE);
			printf("%d", carte->valeur);
			couleurOff();
		break;
	}
		printf("\n");
}
/*
int main(){
	Carte carte;
	int i;
	int j;
	for(i = 1;i<5;i++){
		carte.couleur = i;
		for(j = 1;j<14;j++){
			carte.valeur = j;
			afficherCarte(&carte);
		}
	}
	printf("ok\n");
}
*/
