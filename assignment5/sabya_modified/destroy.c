#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define SEMKEY 89

int main(int argc, char const *argv[])
{
	int semid = semget(SEMKEY, 7, 0666);
	semctl(semid, 0, IPC_RMID);
	return 0;
}