#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <utmp.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define PID_ARRAY 0
#define MSG_SEM 1
#define SEM_KEY 666
#define MSG_KEY 777
#define SHM_KEY_1 887
#define SHM_KEY_2 998
#define MSG_SIZE 5000

//global variables for convenience of use in functions
int *pid;
char *msg;
char tmpMsg[5000];
int msgqid;
int semid;
int shmid1;
int shmid2;

typedef struct mymsgbuf 
{
	long mtype;
	char data[5000];
}mymsgbuf;


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


void initialize()
{
	//initialize msg queue
	msgqid = msgget(MSG_KEY,0666);
	if(msgqid==-1)
	{
		perror("error creating msg queue");
	}

	if ((semid = semget(SEM_KEY,2,0666)) == -1) {
		perror("ranger : Error in creating semaphore set\n");
		exit(1);
	}
	if((shmid1 = shmget(SHM_KEY_1,5000,0666))==-1)
	{
		perror("Error getting shared memory for pid array");
	}
	if((shmid2 = shmget(SHM_KEY_2,5000,0666))==-1)
	{
		perror("Error getting shared memory for the msg");
	}
	if((pid = (int *)(shmat(shmid1,0,0)))== 0)
	{
		perror("Memory attachment error");
		exit(1);
	}
	if((msg = (char *)(shmat(shmid2,0,0)))== 0)
	{
		perror("Memory attachment error");
		exit(1);
	}
	strcpy(tmpMsg,".");
}

void waitOnSemop(int semid,int semnum)
{
	int rv;
	struct sembuf mybuf;
	mybuf.sem_num = semnum;
	mybuf.sem_op = -2;
	mybuf.sem_flg = 0;
	if ((rv = semop(semid, &mybuf, 1)) == -1) {
		perror("client : error in writing ");
		exit(1);
	}
}

void getMessage()
{
	printf("\n< Ctrl+C is pressed >\n");
	printf("--- Enter your message\n");
	char buf[5000];
	char *p;
	if(fgets(buf, sizeof(buf), stdin)!=NULL)
    {
        if ((p = strchr(buf, '\n')) != NULL)
        *p = '\0';
    }
	strcpy(tmpMsg,buf);
}
void exitChild()
{
	printf("Receiver is exiting. It was nice chatting\n");
	exit(0);
}
int main(int argc, char const *argv[])
{
	int rval,i;
	char ser[10];
	rval = access("ser.txt",F_OK);	
	signal(SIGINT,getMessage);
	char *p;
	printf("My pid is %d\n",getpid());
	if(rval!=0)
	{
		perror("File read error");
		exit(1);
	}
	initialize();
	int server_pid = atoi(ser);
	int mypid = getpid();
	sprintf(ser,"%d",mypid);
	pid_t cpid = fork();
	if(cpid==0)
	{
		//child process to receive the messages
		mymsgbuf recmsg;
		int parent_pid = getppid();
		signal(SIGINT,SIG_IGN);
		signal(SIGUSR1,exitChild);
		int i,rv_bytes;
		down(semid,PID_ARRAY);
		for(i=0;i<100;i++)
		{
			if(pid[i]==-1)
			{
				pid[i]=parent_pid;
				break;
			}
		}
		up(semid,PID_ARRAY);
		//receive from msg queue
		while (1) 
		{
        	rv_bytes = msgrcv(msgqid, &recmsg, MSG_SIZE, parent_pid, 0);
        	if(rv_bytes==-1)
	        {
	            perror("Queue Connection error");
	            exit(1);
	        }
	        else
	        {
	        	printf("--- Received Message:\"%s\"\n",recmsg.data);
	        }
	    }
        exit(0);    
    }
    //parent process	
    

    while(1)
    {
    	if(strcmp(tmpMsg,".")!=0)
    	{
    		if(strcmp(tmpMsg,"bye")==0)
    		{
    			//the process wants to exit. I need to free up the values.
    			kill(cpid,SIGUSR1);//send the signal to child to exit.
    			rval = 0;
    			down(semid,PID_ARRAY);
    			for(i=0;i<100;i++)
    			{
    				if(pid[i]!=-1)
    				{
    					if(pid[i]==mypid)
    					{
    						pid[i]=-1;
    					}
    					else
    					{
    						rval++;
    					}
    				}
    			}
    			up(semid,PID_ARRAY);
    			if(rval==0)
    			{
    				//none of the children are present
    				waitOnSemop(semid,MSG_SEM);
    				strcpy(msg,"*");
    				down(semid,MSG_SEM);
    				strcpy(tmpMsg,".");
    				exit(0);
    				//send this message
    			}
    			else
    			{
    				printf("I am going. Have a nice chat\n");
    				exit(0);
    			}
    		}
    		else
    		{
    			p = getlogin();
    			waitOnSemop(semid,MSG_SEM);
    			strcpy(msg,p);
    			strcat(msg,"/");
    			strcat(msg,ser);
    			strcat(msg,": ");
    			strcat(msg,tmpMsg);
    			down(semid,MSG_SEM);
    			strcpy(tmpMsg,".");
    		}
    	}
    }
	return 0;
}