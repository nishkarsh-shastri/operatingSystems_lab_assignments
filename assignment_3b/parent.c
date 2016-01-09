//Header files for system based available functions
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <signal.h>

//Macros for the parent function used for interprocess communication
#define CHILD_READY 100
//Two signals will be used by parent to communticate with the child. Namely SIGUSR1 and SIGUSR2

int main()
{
	srand(time(NULL)); // randomize seed
	int pipe_child_parent[2][2];
	pid_t pid[2];

	//for child i
	//parent has read pipe at pipe_child_parent[i][0]
	//child i has write pipe at pipe_child_parent[i][1]

	//useful variables
	int i;
	int status; //get the status of child closing
	int val,len; //some extra variables
	char childarg1[80]; //the read pipe fd for child
	float child1_score = 0;
	float child2_score = 0;

    int reply[2];

    for(i=0;i<2;i++)
    {
    	if(pipe(pipe_child_parent[i])<0)
    	{
    		perror("Failed to allocate pipes");
            exit(EXIT_FAILURE);
    	}
    }
    //spawn two children
	for(i=0;i<2;i++)
	{
		pid[i] = fork();
		if(pid[i]==0)
		{
			//close the pipes related to parent
			close(pipe_child_parent[i][0]);
        	sprintf(childarg1,"%d",pipe_child_parent[i][1]);//give the child write pipe as argument
        	char *cmd[] = {"./child",childarg1,0};//create the command
        	execvp("./child",cmd);    //execute the process
		}
	}
	//we can wait here for the response of both the children to see whether they are ready or not.
	//lets use pipe handling to get there response it would be fun
	printf("we have created\n");
	reply[0]=reply[1]=-1;
	for(i=0;i<2;i++)
	{
		while(reply[i]==-1)//read the child's response
		{
			len = read(pipe_child_parent[i][0], &reply[i], sizeof(reply[i]));
		}
		if(reply[i]==CHILD_READY)
		{
			printf("-- We have established communication with player %d\n",i+1);
		}
	}

	/* The following code is used
	1 -- paper
	2 -- scissor
	3 -- rock
	*/
	//Parent sends the ready signal - SIGUSR1 to the children 
	while(child1_score<10 && child2_score<10)
	{
		for(i=0;i<2;i++)
			reply[i]=-1;
		// since none of the child is the winner, lets ask both of them for their choice.
		for(i=0;i<2;i++)
		{
			kill(pid[i],SIGUSR1);//send signal to the children to send their random values
			while(reply[i]==-1)//read the child's response
			{
				len = read(pipe_child_parent[i][0], &reply[i], sizeof(reply[i]));
				//printf("%d\n",reply[i]);
			}
			sleep(3); 
		}

		switch(reply[0])
		{
			case 1:
				if(reply[1]==1)
				{
					printf("-- Both of them chose paper -- Nobody won\n");
					child1_score+=0.5;
					child2_score+=0.5;
					printf("-- Current Score: Player 1 = %f\t Player 2 = %f\n",child1_score,child2_score);
					break;
				}
				else if(reply[1]==2)
				{
					printf("-- Player 2 chose scissors and Player 1 chose paper -- Player 2 won\n");
					child1_score+=0;
					child2_score+=1;
					printf("-- Current Score: Player 1 = %f\t Player 2 = %f\n",child1_score,child2_score);
					break;
				}
				else if(reply[1]==3)
				{
					printf("-- Player 2 chose rock and Player 1 chose paper -- Player 1 won\n");
					child1_score+=1;
					child2_score+=0;
					printf("-- Current Score: Player 1 = %f\t Player 2 = %f\n",child1_score,child2_score);
					break;
				}
			case 2:
				if(reply[1]==1)
				{
					printf("-- Player 2 chose paper and Player 1 chose scissors -- Player 1 won\n");
					child1_score+=1;
					child2_score+=0;
					printf("-- Current Score: Player 1 = %f\t Player 2 = %f\n",child1_score,child2_score);
					break;
				}
				else if(reply[1]==2)
				{
					printf("-- Both of them chose scissors -- Nobody won\n");
					child1_score+=0.5;
					child2_score+=0.5;
					printf("-- Current Score: Player 1 = %f\t Player 2 = %f\n",child1_score,child2_score);
					break;
				}
				else if(reply[1]==3)
				{
					printf("-- Player 2 chose rock and Player 1 chose scissors -- Player 2 won\n");
					child1_score+=0;
					child2_score+=1;
					printf("-- Current Score: Player 1 = %f\t Player 2 = %f\n",child1_score,child2_score);
					break;
				}
			case 3:
				if(reply[1]==1)
				{
					printf("-- Player 2 chose paper and Player 1 chose rock -- Player 2 won\n");
					child1_score+=0;
					child2_score+=1;
					printf("-- Current Score: Player 1 = %f\t Player 2 = %f\n",child1_score,child2_score);
					break;
				}
				else if(reply[1]==2)
				{
					printf("-- Player 2 chose scissors and Player 1 chose rock -- Player 1 won\n");
					child1_score+=1;
					child2_score+=0;
					printf("-- Current Score: Player 1 = %f\t Player 2 = %f\n",child1_score,child2_score);
					break;
				}
				else if(reply[1]==3)
				{
					printf("-- Both of them chose rock -- Nobody won\n");
					child1_score+=0.5;
					child2_score+=0.5;
					printf("-- Current Score: Player 1 = %f\t Player 2 = %f\n",child1_score,child2_score);
					break;
				}
		}
	}

	//now decide the winner
	if(child1_score != child2_score)
	{
		if(child1_score>child2_score)
		{
			printf("-- Player 1 won the game quite clearly\n");
		}
		else
		{
			printf("-- Player 2 won the game quite clearly\n");
		}
	}
	else
	{
		printf("The scores are tied. We need to resolve the tie amicably\n");
		//lets use random numbers to find the winner
		reply[0]= rand()%1000;
		reply[1]= rand()%1000;
		if(reply[0]>reply[1])
		{
			printf("-- Player 1 won the tie\n");
		}
		else
		{
			printf("-- Player 2 won the tie\n");
		}

	}

	//lets send kill signal to both the children
	printf("----- Parent sends kill signals to all children\n");
	for(i=0;i<2;i++)
	{
		kill(pid[i], SIGUSR2);
	}
	for(i=0;i<2;i++)//close the pipes associated with parent
	{
		close(pipe_child_parent[i][0]);
    }
    for(i=0;i<2;i++)//wait for all the children to exit
    {
    	waitpid(pid[i],&status,0);
    	printf("----- Player %d terminates\n",i+1);
    }
    return 0;//exit the parent process


}