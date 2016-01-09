#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//using fork to print the n fibonacci numbers
int main ()
{
	//
   int n, a, b,i,status,next;
   int mypid,parpid,pid;

   printf("n = ");
   scanf("%d",&n);
   a = 1;
   b = 1;//the two terms used for fibonacci

   mypid = getpid();
   parpid = getppid();
   printf("Main Parent: PID = %u, PPID = %u\n", mypid, parpid);

   //going for a loop condition
   for(i=1;i<=n;i++)
   {
   	 pid = fork();
   	 if(pid==0)
   	 {
   	 	//this is the child process
   	 	mypid = getpid();
        parpid = getppid();
        printf("\tChild %d: PID = %u, PPID = %u\n",i, mypid, parpid);
        if(i==1 || i == 2)
        {
        	printf("\t\tThe fibonacci term %d = 1\n",i);
        }
        else
        {
        	printf("\t\tThe fibonacci term %d = %d\n",i,a+b);
        }
        i++;
        exit(i);
   	 }
   	 else
   	 {
   	 	//the parent process
   	 	waitpid(pid,&status,0);
   	 	printf("Exiting the process with pid = %d\n",pid);
   	 	mypid = getpid();
   		parpid = getppid();
   		if(i>2){
   			next = b;
   			b = a+b;
   			a = next;
   		}
   	 }
   }

   //printf("Process PID = %u: n = %d\n", mypid, n);

   exit(0);
}