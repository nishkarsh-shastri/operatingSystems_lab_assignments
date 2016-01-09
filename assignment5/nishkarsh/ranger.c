#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>

typedef struct shareddata {
	int Leating[3], Jeating[3], Rproducing[3];
	int Favailable[3];
	int nextpit, Lwaiting, Jwaiting, Rwaiting;
} shareddata;


int semid,shmid;
struct shareddata *shared;

void block (int semnum) {
	int rv;
	struct sembuf mybuf;
	mybuf.sem_num = semnum;
	mybuf.sem_op = -1;
	mybuf.sem_flg = 0;
	if ((rv = semop(semid, &mybuf, 1)) == -1) {
		perror("ranger : Error in block()\n");
		exit(1);
	}
}

void wake (int semnum) {
	int rv;
	struct sembuf mybuf;
	mybuf.sem_num = semnum;
	mybuf.sem_op = 1;
	mybuf.sem_flg = 0;
	if ((rv = semop(semid, &mybuf, 1)) == -1) {
		perror("ranger : Error in wake()");
		exit(1);
	}
}

void acquire (int semnum) {
	int rv;
	struct sembuf mybuf;
	mybuf.sem_num = semnum;
	mybuf.sem_op = -1;
	mybuf.sem_flg = 0;
	if ((rv = semop(semid, &mybuf, 1)) == -1) {
		perror("ranger : Error in acquire()\n");
		exit(1);
	}
}

void release (int semnum) {
	int rv;
	struct sembuf mybuf;
	mybuf.sem_num = semnum;
	mybuf.sem_op = 1;
	mybuf.sem_flg = 0;
	if ((rv = semop(semid, &mybuf, 1)) == -1) {
		perror("ranger : Error in release()");
		exit(1);
	}
}


int main(int argc, char *argv[]) {

	int lion = 0, jackal = 1, ranger = 2, lock[3], glock = 6;
	lock[0] = 3; lock[1] = 4; lock[2] = 5;
	int n, i = 0, visited = 0, j = 0, rv;
	srand((unsigned int)time(NULL));
	n = rand()%3;
	visited = 0;

	if ((semid = semget(666, 7, 0666)) == -1) {
		perror("lion : Error in accessing semaphore set\n");
		exit(1);
	}

	if ((shmid = shmget(999, sizeof(shareddata), 0666)) == -1) {
		perror("lion : Error in accessing shared memory\n");
		exit(1);
	}

	if ((shared = (shareddata*)(shmat(shmid, 0, 0))) == 0) {
		perror("lion : Error in attaching process\n");
		exit(1);
	}

	while (1) {

		printf("Ranger trying to access meat pit %d\n", n + 1);
		acquire(lock[n]);
		if ( shared->Jeating[n] == 0 && shared->Leating[n] == 0 && shared->Favailable[n] < 40) {

			printf("Ranger is adding 5 points to meat pit %d\n", n + 1);
			shared->Rproducing[n] = 1;
			shared->Favailable[n] += 5;
			release(lock[n]);

			sleep(5);//time to add the food
			
			acquire(lock[n]);
			shared->Rproducing[n] = 0;
			j++;
			printf("Ranger left meat pit %d\n", n + 1);
			printf("Food in pit %d = %d\n", n + 1, shared->Favailable[n]);
			acquire(glock);
			shared->nextpit = n;
			printf("Ranger gives signal to wait queue of all meat pits\n");
			for ( i = 0; i < shared->Jwaiting ; i++)
				wake(jackal);
			for ( i = 0; i < shared->Lwaiting ; i++)
				wake(lion);
			release(glock);
			release(lock[n]);
		}
		else {
			release(lock[n]);
			printf("Ranger denied access to meat pit %d\n", n + 1);
			visited++;
			if ( visited < 3) {
				n = (n + 1) % 3;
			}
			else {
				visited = 0;
				acquire(glock);
				shared->Rwaiting++;
				printf("Ranger waiting on pit %d\n", n + 1);
				release(glock);
				block(ranger);
				acquire(glock);
				shared->Rwaiting--;
				n = shared->nextpit;
				release(glock);
			}
			continue;
		}

		n = rand()%3;

	}

	if ((rv = shmdt((const void*)shared)) == -1) {
		perror("ranger : Error in detaching process from shared memory\n");
		exit(1);
	}

	return 0;

}