#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

typedef struct mydatatype {
	int x;
	int y[3];
}mydatatype;

typedef struct shareddata {
	int Leating[3], Jeating[3], Rproducing[3];
	int Favailable[3];
	int nextpit, Lwaiting, Jwaiting, Rwaiting;
} shareddata;

int main(int argc, char const *argv[])
{
	int id = shmget(999, sizeof(mydatatype), 0666), i;
	shareddata* buf = (shareddata*)shmat(id, 0, 0);
	for (i = 0; i < 3; ++i) 
		printf("%d \n", buf->Favailable[i]);
	shmdt((const void*)buf);
	return 0;
}
