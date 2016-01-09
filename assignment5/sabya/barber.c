#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

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
	int semid, shmid, rv, *waiting;
	unsigned short array[3];

	if ((semid = semget(66, 3, IPC_CREAT | 0666)) == -1) {
		perror("barber : Error in creating semaphore set\n");
		exit(1);
	}

	if ((shmid = shmget(99, sizeof(int), IPC_CREAT | 0666)) == -1) {
		perror("barber : Error in creating shared memory\n");
		exit(1);
	}

	array[CUSTOMER] = 0;
	array[BARBER] = 0;
	array[MUTEX] = 1;

	if ((rv = semctl(semid, 0, SETALL, array)) == -1) {
		perror("barber : Error in initializing semaphores\n");
		exit(1);
	}

	if ((waiting = (int*)(shmat(shmid, waiting, 0))) == 0) {
		perror("barber : Error in attaching process to shared memory\n");
		exit(1);
	}

	*waiting = 0;

	while (1) {
		printf("barber : Will sleep if no customer\n");
		down(semid, CUSTOMER);
		printf("barber : New Customer has arrived\n");
		down(semid, MUTEX);
		*waiting = *waiting - 1;
		up(semid, BARBER);
		up(semid, MUTEX);
		printf("barber : Cutting Hair\n");
		sleep(10);
	}

	if ((rv = semctl(semid, 0, IPC_RMID)) == -1) {
		perror("barber : Error in removing semaphore set\n");
		exit(1);
	}

	if ((rv = shmdt((const void*)waiting)) == -1) {
		perror("barber : Error in detaching process from shared memory\n");
		exit(1);
	}

	return 0;
}