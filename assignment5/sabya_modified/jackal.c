#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>

#define LIONS 100
#define JACKALS 100
#define SEMKEY 89
#define SHMKEY 98

typedef struct sharedmem {
	int Leating[3], Jeating[3], Rproducing[3], Favailable[3];
	int nextpit, Lwaiting, Jwaiting, Rwaiting, seq;
	int tm, ltime[LIONS], jtime[JACKALS], rtime; 
}sharedmem;

int semid;

int cmp(const void *a, const void *b) {
	return *(int*)a - *(int*)b;
}

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
		perror("usage : ./jackal number_of_jackals food_per_jackal");
	}

	sharedmem *data;
	int shmid, nJ, k, jackals = 0, j = 0, n, i, visited = 0;
	int ltime[LIONS], lions, rtime;
	int lock[3], lion = 0, jackal = 1, ranger = 2, glock = 6;
	lock[0] = 3; lock[1] = 4; lock[2] = 5;

	nJ = atoi(argv[1]);
	k = atoi(argv[2]);

	if ((semid = semget(SEMKEY, 7, 0666)) == -1) {
		perror("lion : Error in accessing semaphore set\n");
		exit(1);
	}

	if ((shmid = shmget(SHMKEY, sizeof(sharedmem), 0666)) == -1) {
		perror("lion : Error in accessing shared memory\n");
		exit(1);
	}

	if ((data = (sharedmem*)(shmat(shmid, 0, 0))) == 0) {
		perror("lion : Error in attaching process\n");
		exit(1);
	}

	while (jackals < nJ) {

		if (!fork()) {
			srand((unsigned int)(jackals + 200));
			n = rand() % 3;
			while (j < k) {
				acquire(lock[n]);
				acquire(glock);
				printf("%3d : Jackal %d requesting access to pit %d\n", data->seq++, jackals + 1, n + 1);
				if (data->Favailable[n] > 0 && data->Rproducing[n] == 0 && data->Leating[n] == 0) {
					printf("%3d : Jackal %d in control of pit %d\n", data->seq++, jackals + 1, n + 1);
					data->Favailable[n]--;
					data->Jeating[n]++;
					release(glock);
					release(lock[n]);

					sleep(5);

					acquire(lock[n]);
					acquire(glock);
					data->Jeating[n]--;
					j++;
					printf("%3d : Jackal %d leaving pit %d\n", data->seq++, jackals + 1, n + 1);

					if (data->Jeating[n] == 0 && data->Favailable[n] > 0) {
						data->nextpit = n;
						printf("%3d : Jackal %d sending wake signal to wait queues of all meat pits\n", data->seq++, jackals + 1);
						if (data->Rwaiting == 0) {
							for (i = 0; i < data->Lwaiting; ++i)
								wake(lion);
						}
						else {
							lions = 0;
							for (i = 0; i < LIONS; ++i) if (data->ltime[i] != -1) ltime[lions++] = data->ltime[i];
							rtime = data->rtime;
							qsort(ltime, data->Lwaiting, sizeof(int), cmp);
							for (i = 0; i < data->Lwaiting; ++i) {
								if (ltime[i] < rtime) wake(lion);
								else {
									wake(ranger);
									break;
								}
							}
							for (; i < data->Lwaiting; ++i) wake(lion);
						}
					}
					release(glock);
					release(lock[n]);
					n = rand() % 3;
				}
				else {
					release(lock[n]);
					printf("%3d : Jackal %d is denied access to pit %d\n", data->seq++, jackals + 1, n + 1);
					visited++;
					if (visited < 3) {
						n = (n + 1) % 3;
						release(glock);
					}
					else {
						visited = 0;
						data->Jwaiting++;
						printf("%3d : Jackal %d waiting at wait queue of pit %d\n", data->seq++, jackals + 1, n + 1);
						data->jtime[jackals-1] = data->tm++;
						release(glock);
						block(jackal);

						acquire(glock);
						data->jtime[jackals-1] = -1;
						data->Jwaiting--;
						n = data->nextpit;
						release(glock);
					}
				}
			}
			exit(0);
		}
		else {
			++ jackals;
		}

	}

	for (i = 0; i < nJ; ++i) wait(0);

	return 0;
}