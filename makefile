#############################################################
# Projet Unix - Bataille
# GROUPE VERWILGHEN - YAKOUB
#
# Etudiant 1 : VERWILGHEM Maxime
# Etudiant 2 : YAKOUB Jacques
# SERIE 1
############################################################

all: Server Client

Server : Server.o messages.o MemoirePartagee.o
	cc -pthread -g -o Server Server.o messages.o MemoirePartagee.o

Client : Client.o messages.o AfficherCarte.o MemoirePartagee.o
	cc -pthread -g -o Client Client.o messages.o AfficherCarte.o MemoirePartagee.o

Server.o : Server.c types.h Server.h
	cc -c Server.c 

Client.o : Client.c types.h Client.h
	cc -c Client.c 

messages.o : messages.c messages.h
	cc -c -g messages.c messages.h

AfficherCarte.o: AfficherCarte.c AfficherCarte.h types.h
	cc -c AfficherCarte.c AfficherCarte.h

MemoirePartagee.o : MemoirePartagee.c MemoirePartagee.h
	cc -pthread -c -g MemoirePartagee.c MemoirePartagee.h
