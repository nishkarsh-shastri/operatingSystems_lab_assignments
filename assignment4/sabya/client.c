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
#include "headers.h"

int up_msgqid, nClients;
pid_t mypid;
char friendList[MAX_CLIENTS - 1][ID_SIZE];


void send() {
    if(nClients > 0) {
        struct mymsgbuf sd_message;
        char chatID[ID_SIZE], data[DATA_SIZE],*p;
        fflush(stdin);
        printf("\nSelect a User...");
        scanf("%s", chatID);
        int i = 0;
        for(i = 0;i<nClients;i++)
        {
            if(strcmp(chatID,friendList[i])==0)
            {
                break;
            }
        }
        if(i==nClients)
        {
            printf("No such user online. Perhaps you can try some other time\n");
            return;
        }
        int c;
        do{
            c = getchar();
        }while(c != EOF && c != '\n');
        printf("Type Message...");
        //scanf("%[^\n]s",data);
        if(fgets(data, sizeof(data), stdin)!=NULL)
        {
            if ((p = strchr(data, '\n')) != NULL)
            *p = '\0';
        }
        strcpy(sd_message.data, data);
        strcpy(sd_message.dtype, "MSG");
        strcpy(sd_message.chatID, chatID);
        sd_message.mtype = mypid;
        if(msgsnd(up_msgqid, &sd_message, MSG_SIZE, IPC_NOWAIT) == -1) {
            perror("Unable to send Message...Try again");
        }
        else
            printf("Message sent...\n");
    }
    else {
        printf("\nNo User available to send Message To...");
    }
}

int main(int argc, char const *argv[])
{
    /* code */
    int down_msgqid, rv_bytes, i;
    char mychatID[ID_SIZE], chatID[ID_SIZE], data[DATA_SIZE], dtype[TYPE_SIZE], time_buf[TIME_SIZE], cmd[20];
    char *friend, *inp, *tofree;
    inp = (char*)malloc(DATA_SIZE);
    friend = (char*)malloc(ID_SIZE);
    struct mymsgbuf rc_message;
    struct mymsgbuf sd_message;
    nClients = 0;
    up_msgqid = msgget(UP, 0666);
    down_msgqid = msgget(DOWN, 0666);

    if (up_msgqid == -1 || down_msgqid == -1){
        perror("Unable to connect to message Queue...");
        exit(1);
    }

    signal(SIGTSTP, send);
    printf("Enter your ChatID : ");
    scanf("%s", mychatID);
    mypid = getpid();
    sd_message.mtype = mypid;
    strcpy(sd_message.dtype, "NEW");
    strcpy(sd_message.chatID, mychatID);
    if(msgsnd(up_msgqid, &sd_message, MSG_SIZE, IPC_NOWAIT) == -1){
        perror("Unable to Connect...Exiting...");
        exit(1);
    }
    printf("You have Logged In...(Press Ctrl-Z to send a message)\n");
    while (1) {
        rv_bytes = msgrcv(down_msgqid, &rc_message, MSG_SIZE, mypid, 0);
        strcpy(dtype, rc_message.dtype);
        if(rv_bytes == -1){
            strcpy(dtype,"INV");            
        }
        
        if (!strcmp(dtype, "LIST")) {
            tofree = inp;
            strcpy(inp, rc_message.data);
            nClients = 0;
            while((friend = strsep(&inp, ",")) != NULL) {
                if(strcmp(friend, "") && strcmp(friend, mychatID)){
                    strcpy(friendList[nClients++], friend);
                }
            }
            free(tofree);
            inp = (char*)malloc(DATA_SIZE);
        }

        else if (!strcmp(dtype, "MSG")) {

            strcpy(data, rc_message.data);
            strcpy(chatID, rc_message.chatID);
            strcpy(time_buf, rc_message.sd_time_buf);
            printf("(%s) %s says : %s\n", time_buf, chatID, data);

        }

        if(nClients > 0) {
            printf("Users Logged In...");
            for (i = 0; i < nClients; ++i) {
                printf("%s", friendList[i]);
                if(i < nClients - 1)printf(",");                
            }
        }
        else 
            printf("No User Logged In...");
        printf("\n");
    }
    return 0;

}
