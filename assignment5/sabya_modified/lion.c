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
		perror("usage : ./lion number_of_lions food_per_lion");
	}

	sharedmem *data;
	int shmid, nL, k, lions = 0, j = 0, n, i, visited = 0;
	int jtime[JACKALS], jackals, rtime;
	int lock[3], lion = 0, jackal = 1, ranger = 2, glock = 6;
	lock[0] = 3; lock[1] = 4; lock[2] = 5;

	nL = atoi(argv[1]);
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

	while (lions < nL) {

		if (!fork()) {
			srand((unsigned int)(lions + 100));
			n = rand() % 3;
			while (j < k) {
				acquire(lock[n]);
				acquire(glock);
				printf("%3d : Lion %d requesting access to pit %d\n", data->seq++, lions + 1, n + 1);
				if (data->Favailable[n] > 0 && data->Rproducing[n] == 0 && data->Jeating[n] == 0) {
					printf("%3d : Lion %d in control of pit %d\n", data->seq++, lions + 1, n + 1);
					data->Favailable[n]--;
					data->Leating[n]++;
					release(glock);
					release(lock[n]);

					sleep(5);

					acquire(lock[n]);
					acquire(glock);
					data->Leating[n]--;
					j++;
					printf("%3d : Lion %d leaving pit %d\n", data->seq++, lions + 1, n + 1);

					if (data->Leating[n] == 0 && data->Favailable[n] > 0) {
						data->nextpit = n;
						printf("%3d : Lion %d sending wake signal to wait queues of all meat pits\n", data->seq++, lions + 1);
						if (data->Rwaiting == 0) {
							for (i = 0; i < data->Jwaiting; ++i)
								wake(jackal);
						}
						else {
							jackals = 0;
							for (i = 0; i < JACKALS; ++i) if (data->jtime[i] != -1) jtime[jackals++] = data->jtime[i];
							rtime = data->rtime;
							qsort(jtime, data->Jwaiting, sizeof(int), cmp);
							for (i = 0; i < data->Jwaiting; ++i) {
								if (jtime[i] < rtime) wake(jackal);
								else {
									wake(ranger);
									break;
								}
							}
							for (; i < data->Jwaiting; ++i) wake(jackal);
						}
					}
					release(glock);
					release(lock[n]);
					n = rand() % 3;
				}
				else {
					release(lock[n]);
					printf("%3d : Lion %d is denied access to pit %d\n", data->seq++, lions + 1, n + 1);
					visited++;
					if (visited < 3) {
						n = (n + 1) % 3;
						release(glock);
					}
					else {
						visited = 0;
						data->Lwaiting++;
						printf("%3d : Lion %d waiting at wait queue of pit %d\n", data->seq++, lions + 1, n + 1);
						data->ltime[lions-1] = data->tm++;
						release(glock);
						block(lion);

						acquire(glock);
						data->ltime[lions-1] = -1;
						data->Lwaiting--;
						n = data->nextpit;
						release(glock);
					}
				}
			}
			exit(0);
		}
		else {
			++ lions;
		}

	}

	for (i = 0; i < nL; ++i) wait(0);

	return 0;
}