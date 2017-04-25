#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include "types.h"
#include "MemoirePartagee.h"
#define MUTEX "/MUTEX"
#define BD "/BD"
int rc = 0; 


int creerSegment() {
	int shmid;
	sem_t * bd;
	sem_t * mutex; 
	key_t clef;

	// créer la clée
	clef = CLE_PUBLIQUE;

	// perms sur segments
	SYS(shmid = shmget(clef,sizeof(Scores),IPC_CREAT|0644));
	if ( (bd = sem_open(BD,  O_CREAT, 0644, 1)) == NULL) {
		perror("SEMAPHORE\n");
		exit(1);
	}
	if ( (mutex = sem_open(MUTEX,  O_CREAT, 0644, 1)) == NULL) {
		perror("SEMAPHORE\n");
		exit(1);
	}
	return shmid;

}

void supprimerSegment() {
	int shmid;
	SYS((shmid = shmget(CLE_PUBLIQUE, sizeof(Scores), IPC_EXCL|0644)));
	SYS(shmctl(shmid,IPC_RMID,NULL));
	sem_t * bd;
	sem_t * mutex;
	if ( (bd = sem_open(BD, IPC_EXCL|0)) == NULL) {
		perror("SEMAPHORE\n");
		exit(1);
	}
	if ( (mutex = sem_open(MUTEX, IPC_EXCL|0)) == NULL) {
		perror("SEMAPHORE\n");
		exit(1);
	}
	SYS(sem_destroy(bd)); 
	SYS(sem_destroy(mutex)); 
}

void lireMemoire(int nombreJoueurs) { 
	sem_t * bd;
	sem_t * mutex;
	if ( (bd = sem_open(BD, IPC_EXCL|0)) == NULL) {
		perror("SEMAPHORE\n");
		exit(1);
	}
	if ( (mutex = sem_open(MUTEX, IPC_EXCL|0)) == NULL) {
		perror("SEMAPHORE\n");
		exit(1);
	}
	Scores * pointer;
	Scores * data;
	sem_wait(mutex);
	rc = rc +1;
	if (rc == 1)
	sem_wait(bd);
	sem_post(mutex);

	// s'attacher au segment
	int shmid;
	
	SYS((shmid = shmget(CLE_PUBLIQUE, sizeof(Scores), IPC_EXCL|0644)));
	
	if ((pointer = shmat(shmid,NULL,0)) == (Scores * ) -1) {
		perror("Pas de mémoire partagée\n");
		exit(1);
	}
	
	// lecture des scores
	printf("Les scores des joueurs\n");
	int i;

	for (i = 0; i < nombreJoueurs ; i++) {
		printf("%s - score : %d \n",pointer->nomJoueurs[i],data->scores[i]);
	}

	// détacher le segment
	SYS(shmdt(pointer));
	
	sem_wait(mutex);
	rc = rc -1;
	if ( rc == 0) {
		sem_post(bd);
	}
	sem_post(mutex);
	
}

void ecrireMemoire(int scoreJoueur ,int indice , char joueur[50]) {
	sem_t * bd;
	sem_t * mutex;
	if ( (bd = sem_open(BD, O_EXCL|0644)) == NULL) {
		perror("SEMAPHORE\n");
		exit(1);
	}
	if ( (mutex = sem_open(MUTEX, O_EXCL|0644)) == NULL) {
		perror("SEMAPHORE\n");
		exit(1);
	}
	sem_wait(bd);
	
	// s'attacher au segment
	int shmid;
	Scores * data;
	
	SYS((shmid = shmget(CLE_PUBLIQUE, sizeof(Scores), IPC_EXCL|0644)));
	
	// obtenir pointer
	if ((data = shmat(shmid,NULL,0)) == (Scores * ) -1) {
		perror("Pas de mémoire partagée\n");
		exit(1);
	}
	
	// écriture données
	strcpy(data->nomJoueurs[indice], joueur);
	data->scores[indice] += scoreJoueur;
	
	sem_post(bd);
}
