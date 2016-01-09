#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>

#define LIONS 100
#define JACKALS 100
#define SEMKEY 89
#define SHMKEY 98

typedef struct sharedmem {
	int Leating[3], Jeating[3], Rproducing[3], Favailable[3];
	int nextpit, Lwaiting, Jwaiting, Rwaiting, seq;
	int tm, ltime[LIONS], jtime[JACKALS], rtime; 
}sharedmem;

void initialize()
{
	unsigned short array[7];
	int i, rv;
	array[0] = array[1] = array[2] = 0;
	array[3] = array[4] = array[5] = array[6] = 1;
	sharedmem *shared;
	int semid, shmid;
	
	if ((semid = semget(SEMKEY, 7, IPC_CREAT | 0666)) == -1) {
		perror("ranger : Error in creating semaphore set\n");
		exit(1);
	}

	if ((shmid = shmget(SHMKEY, sizeof(sharedmem), IPC_CREAT | 0666)) == -1) {
		perror("ranger : Error in creating shared memory\n");
		exit(1);
	}

	
	if((rv = semctl(semid, 0, SETALL, array)) == -1) {
		perror("SETALL: Error in setting up sub-semaphores");
		exit(1);
	}

	if((shared = (sharedmem *)(shmat(shmid,0,0)))== 0)
	{
		perror("Memory attachment error");
		exit(1);
	}

	for (i = 0; i < 3; i++) {
		shared->Leating[i] = 0;
		shared->Jeating[i] = 0;
		shared->Rproducing[i] = 0;
		shared->Favailable[i] = 0;
	}
	shared->nextpit = shared->Lwaiting = shared->Jwaiting = shared->Rwaiting = 0;
	shared->seq = shared->tm = 0;
	memset(shared->ltime, -1, sizeof(shared->ltime));
	memset(shared->jtime, -1, sizeof(shared->jtime));
	shared->rtime = -1;
}

int main(int argc, char const *argv[])
{
	initialize();
	return 0;
}