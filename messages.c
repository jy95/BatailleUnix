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
#include "types.h"
#include "messages.h"


void str2msg(char * string, Message* message){

	char messageText[256];
	strncpy(messageText,string,255);
	message->type = atoi(strtok(messageText,";"));
	strncpy(message->message, strtok(NULL, ";"),256);
	char * buf;
	int i = 0;
	while(1){
		buf = strtok(NULL, ";");
		if(buf==NULL)break;
		message->cartes[i].valeur = atoi(buf);
		buf = strtok(NULL, ";");
		if(buf==NULL)break;
		message->cartes[i].couleur = atoi(buf);
		i++;
	}
	
}

char* msg2str(Message* message){
	char * output;
	if ((output = (char*)malloc(sizeof(char)*255)) == NULL ){
		perror("erreur malloc");
	}
	sprintf(output, "%d", message->type);
	strcat(output, ";");
	strcat(output, message->message);
	strcat(output, ";");
	int i;
	for(i = 0;i < 4;i++){
		if(message->cartes[i].valeur > 0 && message->cartes[i].valeur < 14){
			sprintf(output, "%s%d", output,message->cartes[i].valeur);
			strcat(output, ";");
			sprintf(output, "%s%d", output, message->cartes[i].couleur);
			strcat(output, ";");
		}
	}

	return output;
}
