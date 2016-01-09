//Header files for system based available functions
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

//Macros for the parent function used for interprocess communication
#define REQUEST 100
#define PIVOT 200
#define LARGE 300
#define SMALL 400
#define READY 500
#define EXIT 600
time_t t;



int main()
{
	srand(time(NULL)); // randomize seed
	int pipe_parent_child[5][2];
	int pipe_child_parent[5][2];
	pid_t pid[5];

	//for child i
	//parent has read pipe at pipe_child_parent[i][0]
	//parent has write pipe at pipe_parent_child[i][1]

	//child i has read pipe at pipe_parent_child[i][0]
	//child i has write pipe at pipe_child_parent[i][1]

	//useful variables
	int i,j,count=0;
	int status;
	int val,len,k;
	int send;
	int receive = 0;
	int id;
	char childarg1[80];
    char childarg2[80];
    int m=0;
    int reply[5];
    int n = 25; //change the value of n for other applications
    //create all the pipes
    for(i=0;i<5;i++)
    {
    	if(pipe(pipe_parent_child[i])<0)
    	{
    		perror("Failed to allocate pipes");
            exit(EXIT_FAILURE);
    	}
    	if(pipe(pipe_child_parent[i])<0)
    	{
    		perror("Failed to allocate pipes");
            exit(EXIT_FAILURE);
    	}
    }
    //spawn five children
	for(i=0;i<5;i++)
	{
		pid[i] = fork();
		if(pid[i]==0)
		{
			//close the pipes related to parent
			close(pipe_child_parent[i][0]);
        	close(pipe_parent_child[i][1]);	
        	sprintf(childarg1,"%d",pipe_parent_child[i][0]);//give the child read pipe as argument
        	sprintf(childarg2,"%d",pipe_child_parent[i][1]);//give the child write pipe as argument
        	char *cmd[] = {"./child",childarg1,childarg2,0};//create the command
        	execvp("./child",cmd);    //execute the process
		}
	}
	
	//close the pipes associated with child processes
	for(i=0;i<5;i++)
	{
		close(pipe_parent_child[i][0]);
        close(pipe_child_parent[i][1]);	
	}

	//send id to all the child processes via their pipes
	for(i=0;i<5;i++)
	{
		send = i;
		write(pipe_parent_child[i][1],&send,sizeof(send));
	}
	//receive ready signals from all the processes
	count = 0;
	for(i=0;i<5;i++)
	{
		while(receive!=READY)//wait to get the ready signal
		{
			len = read(pipe_child_parent[i][0], &receive, sizeof(receive));			
		}
		receive = 0;
	}
	printf("----- Parent READY\n");

	
	m = 0;//initializations
	k = n/2;
	int pivot=-1;
	int sum = 0;

	while(1)
	{
		
		pivot=-1;
		while(pivot==-1)//get the pivot element
		{
			id = rand()%5;
			send = REQUEST;
			printf("----- Parent sends REQUEST to Child %d\n",id+1);
			write(pipe_parent_child[id][1],&send,sizeof(send));
			len = read(pipe_child_parent[id][0], &receive, sizeof(receive));			
			pivot = receive;		
		}
		printf("----- Parent broadcasts pivot %d to all children\n",pivot);
		for(i=0;i<5;i++)//broadcas pivot to all the children
		{
			send = PIVOT;
			write(pipe_parent_child[i][1],&send,sizeof(send));
			send = pivot;
			write(pipe_parent_child[i][1],&send,sizeof(send));
		}
		
		for(i=0;i<5;i++)
		{
			reply[i] = -1;
		}
		for(i=0;i<5;i++)//get the count of integers greater than pivot from all the children
		{
			while(reply[i]==-1)
				len = read(pipe_child_parent[i][0], &reply[i], sizeof(reply[i]));
			
		}

		sum = 0;
		for(i=0;i<5;i++)
			sum+=reply[i];
		printf("----- Parent: m = %d + %d + %d + %d + %d = %d\n", reply[0],reply[1],reply[2],reply[3],reply[4],sum);
		m = sum;
		//add and compare the values returned from all the processes

		if(m==k)//the pivot used is the median. report it
		{	
			printf("-----  %d = %d.Median Found!\n",m,k);
			break;
		}
		if(m>k)//we have to remove smaller elements
		{
			printf("-----  %d > %d.Median not found!\n",m,k);
			for(i=0;i<5;i++)
			{
				send = SMALL;
				write(pipe_parent_child[i][1],&send,sizeof(send));
				send = pivot;
				write(pipe_parent_child[i][1],&send,sizeof(send));
			}
			for(i=0;i<5;i++)//wait to get acknowledgement of removal from all the children
			{
				read(pipe_child_parent[i][0], &receive, sizeof(receive));
			}
		}
		if(m<k)//we have to remove the larger elements
		{
			printf("----- %d < %d.Median not Found!\n",m,k);
			for(i=0;i<5;i++)
			{
				send = LARGE;
				write(pipe_parent_child[i][1],&send,sizeof(send));
				send = pivot;
				write(pipe_parent_child[i][1],&send,sizeof(send));
			}
			for(i=0;i<5;i++)
			{
				read(pipe_child_parent[i][0], &receive, sizeof(receive));
			}
			k = k-m;//update the value of k

		}


		
	}
	printf("----- Median is %d\n",pivot);//median found
	printf("----- Parent sends kill signals to all children\n");
	for(i=0;i<5;i++)
	{
		send = EXIT;//send exit signal to all the children
		write(pipe_parent_child[i][1],&send,sizeof(send));
	}
	for(i=0;i<5;i++)//close the pipes associated with parent
	{
		close(pipe_child_parent[i][0]);
    	close(pipe_parent_child[i][1]);	
    }
    for(i=0;i<5;i++)//wait for all the children to exit
    {
    	waitpid(pid[i],&status,0);
    }
    return 0;//exit the parent process

}


