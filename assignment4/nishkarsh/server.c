#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>


#define UP 			8
#define DOWN 		9
#define TYPE_SIZE 	10
#define DATA_SIZE 	256
#define ID_SIZE 	20
#define TIME_SIZE	20
#define MSG_SIZE 	300
#define MAX_CLIENTS 25

typedef struct mymsgbuf {
	long mtype;
	char dtype[TYPE_SIZE];
	char data[DATA_SIZE];
	char chatID[ID_SIZE];
	char sd_time_buf[TIME_SIZE];
}mymsgbuf;

int up_msgqid, down_msgqid;

typedef struct ele{
	pid_t pid;
	char chatID[ID_SIZE];
}ele;

void removeMessageQueue()
{
	if (msgctl(up_msgqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }
    if (msgctl(down_msgqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }
    exit(0);
}

int main(int argc, char const *argv[])
{
	int rv_bytes, nClients = 0, i, flag;
	char dtype[TYPE_SIZE], data[DATA_SIZE], chatID[ID_SIZE], sd_time_buf[TIME_SIZE];
	pid_t pid;
	time_t sd_time;
	struct mymsgbuf rc_message;
	struct mymsgbuf sd_message;
	struct msqid_ds buf;
	struct ele mapping[MAX_CLIENTS], element;
	signal(SIGINT,removeMessageQueue);
	//create the msgq
	up_msgqid = msgget(UP,IPC_CREAT|0666);
	down_msgqid = msgget(DOWN,IPC_CREAT|0666);

	//check for the errors blah blah blah

	//go to a loop and do the work as asked 

	while(1)
	{
		rv_bytes = msgrcv(up_msgqid,&rc_message,MSG_SIZE,0,0);
		strcpy(dtype,rc_message.dtype);
		//check for the msg type
		if(!strcmp(dtype,"NEW"))//its a new client
		{
			strcpy(element.chatID,rc_message.chatID);
			element.pid = rc_message.mtype;
			mapping[nClients++]=element;
			printf("Client PID = %d, ChatID = %s has just logged in...\n", element.pid, element.chatID);
			printf("%d Clients Online\n", nClients);
			strcpy(data, "");//msg that I will send
			for (i = 0; i < nClients; ++i) {
				strcat(data, mapping[i].chatID);
				strcat(data, ",");//separate the client chat Id with a ','
			}

			strcpy(sd_message.dtype, "LIST"); //broadcast the list of the chatIDs to all the clients
			strcpy(sd_message.data, data); //Append the message with the structure

			for (i = 0; i < nClients; ++i) {
				sd_message.mtype = mapping[i].pid; //set the mstype to  the pid of the client so that only individual clients can receive that msg
				flag = msgsnd(down_msgqid, &sd_message, MSG_SIZE, IPC_NOWAIT);//send it via the down msg queue
			}
			printf("Updated Client List sent to all Clients...\n\n");
		}

		else if (!strcmp(dtype, "MSG")) {

			strcpy(data, rc_message.data);
			pid = rc_message.mtype;
			for (i = 0; i < nClients; ++i) {
				if(mapping[i].pid == pid) {
					strcpy(chatID, mapping[i].chatID);
					break;
				}
			}

			printf("A message received from PID = %d, ChatID = %s\n", pid, chatID);
			msgctl(up_msgqid, IPC_STAT, &buf);
			printf("No. of Messages in UP-Queue : %lu\n", buf.msg_qnum);
			sd_time = buf.msg_stime;
			strftime(sd_time_buf, TIME_SIZE, "%H:%M:%S", localtime(&sd_time));

			strcpy(sd_message.sd_time_buf, sd_time_buf);
			strcpy(sd_message.chatID, chatID);
			strcpy(chatID, rc_message.chatID);
			for (i = 0; i < nClients; ++i) {
				if(!strcmp(mapping[i].chatID, chatID)){
					pid = mapping[i].pid;
					break;
				}
			}
			sd_message.mtype = pid;
			strcpy(sd_message.dtype,"MSG");
			strcpy(sd_message.data, rc_message.data);

			msgsnd(down_msgqid, &sd_message, MSG_SIZE, IPC_NOWAIT);
			printf("A message sent to PID = %d, ChatID = %s\n", pid, rc_message.chatID);
			msgctl(down_msgqid, IPC_STAT, &buf);
			printf("No. of Messages in DOWN-Queue : %lu\n\n", buf.msg_qnum);

		}
	}
	return 0;
}