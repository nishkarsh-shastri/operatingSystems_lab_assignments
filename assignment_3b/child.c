/* The Library headers for implementing functions */
#include <stdio.h> /*typically in /usr/include*/
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define CHILD_READY 100

int id;//the child id
int child_write;

void exit_child()
{
    //Exit from the process. All work done.
    close(child_write);
    exit(EXIT_SUCCESS);
}

void generateAndSendRandomNumber()
{
	srand((unsigned int) time(NULL));//randomize seed
	int num = rand()%3 + 1;
	write(child_write, &num, sizeof(num));//send ready signal
	printf("-- I am sending %d as my move\n",num);
}

int main(int argc,char *argv[])
{
    srand((unsigned int) time(NULL));//randomize seed

    child_write = atoi(argv[1]);//get the child write pipe as argument

    int len,val,send;//variables for communication
    pid_t pid;
    int i;//used for iteration purposes
    send = CHILD_READY;
    printf("----- Player sends READY\n");//child is ready
    write(child_write, &send, sizeof(send));//send ready signal
    struct sigaction sa_gen;
    struct sigaction sa_end;

    sa_gen.sa_handler = generateAndSendRandomNumber;
    sa_gen.sa_flags = 0;
    sigemptyset(&sa_gen.sa_mask);

    sa_end.sa_handler = exit_child;
    sa_end.sa_flags = 0;
    sigemptyset(&sa_end.sa_mask);    

    while(1)
    {
    	if(sigaction(SIGUSR1, &sa_gen, NULL) == -1) 
    	{
        	perror("sigaction");
        	exit(1);
    	}
    	if(sigaction(SIGUSR2, &sa_end, NULL) == -1) 
    	{
        	perror("sigaction");
        	exit(1);
    	}
    }


}