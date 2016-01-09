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
} shareddata;

void block (int semnum) {
	int rv;
	struct sembuf mybuf;
	mybuf.sem_num = semnum;
	mybuf.sem_op = -1;
	mybuf.sem_flg = 0;
	if ((rv = semop(semid, &mybuf, 1)) == -1) {
		perror("lion : Error in block()\n");
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
		perror("lion : Error in wake()");
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
		perror("lion : Error in acquire()\n");
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
		perror("lion : Error in release()");
		exit(1);
	}
}


int main(int argc, char const *argv[])
{
	if (argc != 3) {
		perror("usage : lions food");
		exit(1);
	}

	int nL, k, visited = 0, n = 1, j = 0, i = 0, rv;
	int lion = 0, jackal = 1, ranger = 2, glock = 6, lock[3], lions = 0;
	lock[0] = 3; lock[1] = 4; lock[2] = 5;
	int shmid;
	shareddata *data;

	nL = atoi(argv[1]);
	k = atoi(argv[2]);
	srand((unsigned int)time(NULL));

	if ((semid = semget(666, 7, 0666)) == -1) {
		perror("lion : Error in accessing semaphore set\n");
		exit(1);
	}

	if ((shmid = shmget(999, sizeof(shareddata), 0666)) == -1) {
		perror("lion : Error in accessing shared memory\n");
		exit(1);
	}

	if ((data = (shareddata*)(shmat(shmid, 0, 0))) == 0) {
		perror("lion : Error in attaching process\n");
		exit(1);
	}

	while (lions < nL) {
		sleep(2);
		if (!fork()) {
			n = rand()%3;
			while (j < k) {

				printf("Lion %d requesting food from meat pit %d\n", lions + 1, n + 1);
				acquire(lock[n]);

				if ( data->Jeating[n] == 0 && data->Rproducing[n] == 0 && data->Favailable[n] > 0) {
					data->Leating[n]++;
					printf("Lion %d in control of meat pit %d\n", lions + 1, n + 1);
					release(lock[n]);
					sleep(5);
					acquire(lock[n]);
					data->Leating[n]--;
					j++;
					data->Favailable[n]--;
					//release(lock[n]);
					printf("Lion %d left meat pit %d\n", lions + 1, n + 1);
				}
				else {
					release(lock[n]);
					printf("Lion %d denied access to meat pit %d\n", lions + 1, n + 1);
					visited ++;
					if (visited < 3) {
						n = (n + 1)%3;
					}
					else {
						visited = 0;
						acquire(glock);
						data->Lwaiting++;
						printf("Lion %d in wait queue of meat pit %d\n", lions + 1, n + 1);
						release(glock);
						block(lion);
						acquire(glock);
						data->Lwaiting--;
						n = data->nextpit;
						release(glock);
					}
					continue;
				}

				//acquire(lock[n]);
				acquire(glock);
				if (data->Leating[n] == 0 && data->Favailable[n] > 0) {
					data->nextpit = n;
					printf("Lion %d gives signal to wait queue of all meat pit\n", lions + 1);
					for (i = 0; i < data->Jwaiting; ++i)
						wake(jackal);
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
				perror("lion : Error in detaching process from shared memory\n");
				exit(1);
			}
			exit(0);
		}

		else {
			lions ++;
		}
	}

	for (i = 0; i < nL; ++i) wait(NULL);
	return 0;
}