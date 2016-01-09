#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <stdlib.h>

int semid;

typedef struct shareddata {
	int Leating[3], Jeating[3], Rproducing[3];
	int Favailable[3];
	int nextpit, Lwaiting, Jwaiting, Rwaiting;
}shareddata;

void block (int semnum) {
	int rv;
	struct sembuf mybuf;
	mybuf.sem_num = semnum;
	mybuf.sem_op = -1;
	mybuf.sem_flg = 0;
	if ((rv = semop(semid, &mybuf, 1)) == -1) {
		perror("jackal : Error in block()\n");
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
		perror("jackal : Error in wake()");
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
		perror("jackal : Error in acquire()\n");
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
		perror("jackal : Error in release()");
		exit(1);
	}
}


int main(int argc, char const *argv[])
{
	if (argc != 3) {
		perror("usage : jackals food");
		exit(1);
	}

	int nJ, k, visited = 0, n = 1, j = 0, i = 0, rv;
	int lion = 0, jackal = 1, ranger = 2, lock[3], glock = 6, jackals = 0;
	lock[0] = 3; lock[1] = 4; lock[2] = 5;
	int shmid;
	struct shareddata *data;

	nJ = atoi(argv[1]);
	k = atoi(argv[2]);
	srand((unsigned int)time(NULL));

	if ((semid = semget(666, 7, 0666)) == -1) {
		perror("jackal : Error in accessing semaphore set\n");
		exit(1);
	}

	if ((shmid = shmget(999, sizeof(struct shareddata), 0666)) == -1) {
		perror("jackal : Error in accessing shared memory\n");
		exit(1);
	}

	if ((data = (struct shareddata*)shmat(shmid, 0, 0)) == NULL) {
		perror("jackal : Error in attaching process\n");
		exit(1);
	}

	while (jackals < nJ) {
		sleep(2);
		if (!fork()) {
			n = rand()%3;
			while (j < k) {

				printf("Jackal %d requesting food from meat pit %d\n", jackals + 1, n + 1);
				acquire(lock[n]);

				if ( data->Leating[n] == 0 && data->Rproducing[n] == 0 && data->Favailable[n] > 0) {
					data->Jeating[n]++;
					printf("Jackal %d in control of meat pit %d\n", jackals + 1, n + 1);
					release(lock[n]);
					sleep(5);
					acquire(lock[n]);
					data->Jeating[n]--;
					j++;
					data->Favailable[n]--;
					//release(lock[n]);
					printf("Jackal %d left meat pit %d\n", jackals + 1, n + 1);
				}
				else {
					release(lock[n]);
					printf("Jackal %d denied access to meat pit %d\n", jackals + 1, n + 1);
					visited ++;
					if (visited < 3) {
						n = (n + 1)%3;
					}
					else {
						visited = 0;
						acquire(glock);
						data->Jwaiting++;
						printf("Jackal %d in wait queue of meat pit %d\n", jackals + 1, n + 1);
						release(glock);
						block(jackal);
						acquire(glock);
						data->Jwaiting--;
						n = data->nextpit;
						release(glock);
					}
					continue;
				}

				//acquire(lock[n]);
				acquire(glock);
				if (data->Jeating[n] == 0 && data->Favailable[n] > 0) {
					data->nextpit = n;
					printf("Jackal %d gives signal to wait queue of all meat pit\n", jackals + 1);
					for (i = 0; i < data->Lwaiting; ++i)
						wake(lion);
					for (i = 0; i < data->Rwaiting; ++i)
						wake(ranger);
					release(glock);
					release(lock[n]);
				}
				else {
					release(lock[n]);
					release(glock);
				}

				n = rand()%3;

			}

			if ((rv = shmdt((const void*)data)) == -1) {
				perror("jackal : Error in detaching process from shared memory\n");
				exit(1);
			}
			exit(0);
		}

		else {
			jackals ++;
		}

	}

	for (i = 0; i < nJ; ++i) wait(NULL);

	return 0;
}