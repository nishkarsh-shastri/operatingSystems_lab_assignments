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

int main(int argc, char const *argv[])
{
	sharedmem *data;
	int shmid, n, i, j, visited = 0;
	int jtime[JACKALS], jackals, ltime[LIONS], lions, rtime;
	int lock[3], lion = 0, jackal = 1, ranger = 2, glock = 6;
	lock[0] = 3; lock[1] = 4; lock[2] = 5;

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

	srand((unsigned int)0);
	n = rand() % 3;
	while (1) {
		acquire(lock[n]);
		acquire(glock);
		printf("%3d : Ranger requesting access to pit %d\n", data->seq++, n + 1);
		if (data->Leating[n] == 0 && data->Jeating[n] == 0 && data->Favailable[n] <= 40) {
			printf("%3d : Ranger in control of pit %d\n", data->seq++, n + 1);
			data->Favailable[n] += 10;
			data->Rproducing[n] ++;
			release(lock[n]);
			release(glock);

			sleep(2);

			acquire(lock[n]);
			acquire(glock);
			data->Rproducing[n]--;
			printf("%3d : Ranger leaving pit %d\n", data->seq++, n + 1);
			data->nextpit = n;
			printf("%3d : Ranger sending wake signal to wait queues of all meat pits\n", data->seq++);
			lions = jackals = 0;
			for (i = 0; i < LIONS; ++i) if (data->ltime[i] != -1) ltime[lions++] = data->ltime[i];
			for (i = 0; i < JACKALS; ++i) if (data->jtime[i] != -1) jtime[jackals++] = data->jtime[i];
			qsort(ltime, data->Lwaiting, sizeof(int), cmp);
			qsort(jtime, data->Jwaiting, sizeof(int), cmp);
			i = j = 0;
			while (i < lions && j < jackals) {
				if (ltime[i] < jtime[j]) {
					wake(lion);
					++i;
				}
				else {
					wake(jackal);
					++j;
				}
			}
			while (i < lions){
				wake(lion);
				++i;
			}
			while (j < jackals){
				wake(jackal);
				++j;
			}
			release(glock);
			release(lock[n]);
			n = rand() % 3;
		}
		else {
			release(lock[n]);
			printf("%3d : Ranger is denied access to pit %d\n", data->seq++, n + 1);
			visited++;
			if (visited < 3) {
				n = (n + 1) % 3;
				release(glock);
			}
			else {
				visited = 0;
				data->Rwaiting++;
				printf("%3d : Ranger waiting at wait queue of pit %d\n", data->seq++, n + 1);
				data->rtime = data->tm++;
				release(glock);
				block(ranger);

				acquire(glock);
				data->rtime = -1;
				data->Rwaiting--;
				n = data->nextpit;
				release(glock);
			}
		}
	}

	return 0;
}