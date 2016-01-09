#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define SHARED_MEM_KEY 999
#define KEY 666


//sub-semaphore in the semaphore set
//  int lion,jackal,ranger,lock[3],glock;
/*
Shared memory variables
Leating[3],Jeating[3],Rproducing[3],FAvailable[3],Lwaiting,Jwaiting,Rwaiting,nextpit
*/




typedef struct shareddata
{
	int Leating[3],Jeating[3],Rproducing[3];
	int Favailable[3];
	int nextpit,Lwaiting,Jwaiting,Rwaiting;
}shareddata;



/*
** initsem() -- more-than-inspired by W. Richard Stevens' UNIX Network
** Programming 2nd edition, volume 2, lockvsem.c, page 295.
*/

//the file descriptors
int semid,shmid;
shareddata *shared;
void initsem()
{

	if ((semid = semget(KEY, 7, IPC_CREAT | 0666)) == -1) {
		perror("CREATE : Error in creating semaphore set\n");
		exit(1);
	}
	if ((shmid = shmget(SHARED_MEM_KEY, sizeof(shareddata), IPC_CREAT | 0666)) == -1) {
		perror("barber : Error in creating shared memory\n");
		exit(1);
	}
	int array[7];
	array[0]=array[1]=array[2]=0;
	array[3]=array[4]=array[5]=array[6]=0;
	if((rv = semctl(semid,0,SETALL,array))==-1)
	{
		perror("SETALL: Error in setting up sub-semaphores");
	}
	if((shared = (shareddata *)(shmat(shmid,shared,0)))== 0)
	{
		perror("Memory attachment error");
	}
	int i = 0;
	for(i=0;i<3;i++)
	{
		shared->Lwaiting[i]=0;
		shared->Jwaiting[i]=0;
		shared->Rproducing[i]=0;
		shared->Favailable[i] = 20;
	}
	shared->nextpit= shared->Lwaiting=shared->Jwaiting=shared->Rwaiting=0;
}


int main()
{
	int n,i=0,visited;
	n = rand()%3;
	visited = 0;
	while(1)
	{
		printf("Ranger trying to access meat pit %d\n",n);
		acquire(lock,n);
		if(shared->Jeating[n]==0 && shared->Leating[n]==0 && shared->Favailable[n]<40)
		{
			printf("Ranger is adding 10 points to meat pit %d\n",n);
			shared->Rproducing[n] = 1;
			shared->Favailable[n]+=10;
			sleep(3);//time to add the food
			shared->Rproducing[n] = 0;
			release(lock,n);
			aquire(glock);
			shared->nextpit = n;
			for(i=0;i<shared->Jwaiting;i++)
			{
				wake(jackal);
			}
			for(i=0;i<shared->Lwaiting;i++)
			{
				wake(lion);
			}
			release(glock);
		}
		else
		{
			release(lock[n]);
			shared->visited++;
			if(shared->visited<3)
			{
				n = (n+1)%3;
				continue; 
			}
			else
			{
				printf("Ranger waiting on pit %d\n",n);
				block(ranger);
				acquire(glock);
				n = shared->next_pit;
				release(glock);
			}
		}
	}
}