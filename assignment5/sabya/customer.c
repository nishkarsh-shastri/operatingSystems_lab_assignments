#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/wait.h>

#define CUSTOMER 0
#define BARBER 1
#define MUTEX 2

void down (int semid, int semnum) {
	int rv;
	struct sembuf mybuf;
	mybuf.sem_num = semnum;
	mybuf.sem_op = -1;
	mybuf.sem_flg = 0;
	if ((rv = semop(semid, &mybuf, 1)) == -1) {
		perror("barber : Error in down() ");
		exit(1);
	}
}

void up (int semid, int semnum) {
	int rv;
	struct sembuf mybuf;
	mybuf.sem_num = semnum;
	mybuf.sem_op = 1;
	mybuf.sem_flg = 0;
	if ((rv = semop(semid, &mybuf, 1)) == -1) {
		perror("barber : Error in up() ");
		exit(1);
	}
}

int main(int argc, char const *argv[])
{
	int semid, shmid, rv, *waiting, nchairs, nCustomers, random, customersArrived = 0, customersExited = 0;
	srand((unsigned int)time(NULL));

	if (argc != 3) {
		perror("customer : usage nchairs nCustomers\n");
		exit(1);
	}

	nchairs = atoi(argv[1]);
	nCustomers = atoi(argv[2]);

	if ((semid = semget(66, 3, 0666)) == -1) {
		perror("customer : Error in accessing semaphore set\n");
		exit(1);
	}

	if ((shmid = shmget(99, sizeof(int), 0666)) == -1) {
		perror("customer : Error in accessing shared memory\n");
		exit(1);
	}

	if ((waiting = (int*)(shmat(shmid, waiting, 0))) == 0) {
		perror("customer : Error in attaching process to shared memory\n");
		exit(1);
	}

	while (customersArrived < nCustomers) {
		if (!fork()) {
			down(semid, MUTEX);
			if (*waiting < nchairs) {
				*waiting = *waiting + 1;
				up(semid, CUSTOMER);
				up(semid, MUTEX);
				printf("customer %d : Waiting for turn\n", customersArrived + 1);
				down(semid, BARBER);
				printf("customer %d : Getting haircut\n", customersArrived + 1);
				sleep(10);
			}
			else { 
				printf("customer %d : No seat ... customer leaving without haircut\n", customersArrived + 1);
				up(semid, MUTEX);
				exit(0);
			}
			printf("customer %d : Haircut done ... leaving shop\n", customersArrived + 1);
			exit(0);
		}
		else {
			random = rand() % 10;
			sleep(random);
			customersArrived ++;
		}
	}

	while (customersExited < nCustomers) {
		wait(0);
		customersExited++;
	}

	if ((rv = shmdt((const void*)waiting)) == -1) {
		perror("customer : Error in detaching process from shared memory\n");
		exit(1);
	}

	return 0;
}