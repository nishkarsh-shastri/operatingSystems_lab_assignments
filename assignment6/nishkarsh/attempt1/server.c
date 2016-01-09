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
#include <signal.h>

#define PID_ARRAY 0
#define MSG_SEM 1
#define SEM_KEY 666
#define MSG_KEY 777
#define SHM_KEY_1 887
#define SHM_KEY_2 998

//global variables for convenience of use in functions
int *pid;
char *msg;
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
		perror("server : Error in down() ");
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
		perror("server : Error in up() ");
		exit(1);
	}
}

void blockMessageSem(int semid,int semnum)
{
	int rv;
	struct sembuf mybuf;
	mybuf.sem_num = semnum;
	mybuf.sem_op = 0;
	mybuf.sem_flg = 0;
	if ((rv = semop(semid, &mybuf, 1)) == -1) {
		perror("server : Error in blocking ");
		exit(1);
	}
}

void releaseMessageSem(int semid,int semnum)
{
	int rv;
	struct sembuf mybuf;
	mybuf.sem_num = semnum;
	mybuf.sem_op = 3;
	mybuf.sem_flg = 0;
	if ((rv = semop(semid, &mybuf, 1)) == -1) 
	{
		perror("server: Error in releasing msg ");
		exit(1);
	}
}

void initialize()
{
	int a[2],rv;
	a[0]=1;
	a[1]=0;
	//initialize msg queue
	msgqid = msgget(MSG_KEY,IPC_CREAT|0666);
	if(msgqid==-1)
	{
		perror("error creating msg queue");
	}

	if ((semid = semget(SEM_KEY,2, IPC_CREAT | 0666)) == -1) {
		perror("ranger : Error in creating semaphore set\n");
		exit(1);
	}

	if((shmid1 = shmget(SHM_KEY_1,5000,IPC_CREAT|0666))==-1)
	{
		perror("Error getting shared memory for pid array");
		exit(1);
	}

	if((shmid2 = shmget(SHM_KEY_2,5000,IPC_CREAT|0666))==-1)
	{
		perror("Error getting shared memory for the msg");
		exit(1);
	}
	if((rv = semctl(semid, 0, SETALL, a)) == -1) {
		perror("SETALL: Error in setting up sub-semaphores");
		exit(1);
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
	int i;
	//initialize your array
	down(semid,PID_ARRAY);
	for(i=0;i<100;i++)
	{
		pid[i]=-1;
	}
	up(semid,PID_ARRAY);
	printf("--- Initialization Complete\n");
}

void show_info( struct utmp *utbufp )
{
	char name[100];
	printf("--- Sending commence notice to %s\n",utbufp->ut_line);
	sprintf(name,"./commence > /dev/%s", utbufp->ut_line);
	system(name);
}

void identifyActiveUsers(int n,const char *users[])
{
	//get the opened terminal list
	struct utmp	current_record;	/* read info into here       */
	int		utmpfd,i;		/* read from this descriptor */
	int		reclen = sizeof(current_record);

	if ( (utmpfd = open(UTMP_FILE, O_RDONLY)) == -1 ){
		perror( UTMP_FILE );	/* UTMP_FILE is in utmp.h    */
		exit(1);
	}

	while ( read(utmpfd, &current_record, reclen) == reclen )
		for(i=1;i<n;i++)
		{
			if(strcmp(current_record.ut_name,users[i])==0)
			{
				show_info(&current_record);
				break;
			}

		}
	close(utmpfd);
	return;			/* went ok */	
}

void destroy()
{
	//remove ser.txt file
	int status;
	status = remove("ser.txt");
 	
	if( status != 0 )
	{
	  printf("Unable to delete the file\n");
	  perror("Error");
	  exit(1);
	}
	//remove msgqueue
	msgctl(msgqid, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID);
	shmctl(shmid1, IPC_RMID, NULL);
	shmctl(shmid2, IPC_RMID, NULL);
	printf("--- Terminating Conference\n");
	exit(0);
	return;
}

void handleMsg(const char *mymsg)
{
	char *p;
	char buf[5000];
	char *tokenlist[5000];
	int numberOfTokens=0,sender_pid;
	mymsgbuf sendToAll;
	strcpy(buf,mymsg);
	if(strcmp(buf,".")==0)
	{
		//no action needed 
		return;
	}
	if(strcmp(buf,"*")==0)
	{
		printf("Ending all the processes\n");
		destroy();
	}	

	p = strtok(buf,"/");
	int i = 0;
	while(p!=NULL && i<2)
	{
		tokenlist[numberOfTokens++]=strdup(p);
		p = strtok(NULL,"/");
		i++;
	}
	strcpy(buf,tokenlist[1]);
	p = strtok(buf,":");
	numberOfTokens=0;
	i=0;
	while(p!=NULL && i<2)
	{
		tokenlist[numberOfTokens++]=strdup(p);
		p = strtok(NULL,":");
		i++;
	}
	sender_pid=atoi(tokenlist[0]);

	//add that to message queue
	down(semid,PID_ARRAY);
	for(i=0;i<100;i++)
	{
		if(pid[i]!=-1 && pid[i]!=sender_pid)
		{
			printf("--- Sending msg to %d\n",pid[i]);
			sendToAll.mtype = pid[i];
			strcpy(sendToAll.data,mymsg);
			msgsnd(msgqid,&sendToAll,5000, IPC_NOWAIT);
		}
	}
	up(semid,PID_ARRAY);
}
int main(int argc, const char *argv[])
{
	int serfile,i;
	char pid_str[10];
	char buf[5000];
	if(argc<2)
	{
		fprintf(stderr, "You need to give atleast one user as command line argumnet\n");
		exit(0);
	}
	pid_t p = getpid();
	initialize();
	identifyActiveUsers(argc,argv);	
	if((serfile=open("ser.txt",O_CREAT|O_EXCL|0666))==-1)
	{	
		perror("Error in opening. File already exists");
		exit(1);
	}
	signal(SIGINT,destroy);
	sprintf(pid_str,"%d",p);
	write(serfile,pid_str,strlen(pid_str));
	releaseMessageSem(semid,MSG_SEM);
	while(1)
	{
		blockMessageSem(semid,MSG_SEM);
		printf("--- Received Message :\"%s\"\n",msg);
		strcpy(buf,msg);
		handleMsg(buf);
		releaseMessageSem(semid,MSG_SEM);
	}
	return 0;
}
