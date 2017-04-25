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

#if ! defined (_SeMAPHORES_H_)
#define _SEMAPHORES_H_


int creerSegment();

void supprimerSegment();

void lireMemoire(int nombreJoueurs);

void ecrireMemoire(int scoreJoueur ,int indice , char joueur[50]);

#endif
