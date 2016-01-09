#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//using this function to get nth fibonacci number using dynamic programming
int calculateFibN(int n)
{
  int *fib = (int*)malloc(n*sizeof(int));
  fib[0]=1;
  fib[1]=1;
  int i;
  for(i=2;i<n;i++)
  {
    fib[i] = fib[i-1]+fib[i-2];
  }
  return fib[n-1];
}


int main ()
{
	//
   int nfib, a, b,i,status,next;
   int mypid,parpid,pid;
   int child_fib;

   printf("nfib = ");
   scanf("%d",&nfib);
   //The array holding the fibonacci numbers
   int *A = (int *)malloc(nfib*sizeof(int));
   //The array holding the pids of all the child processes calculating the fibonacci number
   int *pid_array = (int*)malloc(nfib*sizeof(int));
   mypid = getpid();
   parpid = getppid();
   printf("\t\tMain Parent: PID = %u, PPID = %u\n", mypid, parpid);

   //going for a loop condition
   for(i=1;i<=nfib;i++)
   {

     pid = fork();

     if(pid==0)
     {
      //child process to calculate the one with the index i
      mypid = getpid();
      parpid = getppid();
      printf("Child %d: PID = %u, PPID = %u\n", i,mypid, parpid);
      //calculate the ith fibonacci number
      child_fib = calculateFibN(i);

      //It will be used for the answer number 3
      printf("\tThe value computed by child %d for term %d is %d\n",i,i,child_fib);
  
      //returning the calculated fiboncci number to the parent
      exit(child_fib);
     }
     else
     {
      //sleep(2);
      //Store the child pid in appropriate index for addressing that pid later on
      pid_array[i] = pid;
     }

   }


   

   for(i=1;i<=nfib;i++)   
   {
    waitpid(pid_array[i],&A[i],0);//retrieve the returned value from the child
    printf("\t\tExiting the process with pid = %d Corresponding to child %d\n",pid_array[i],i);
    A[i]>>=8;//Retrieving the encoded value returned
  }

    sleep(5);
  //printing the fibonacci numbers obtained
    printf("\nThe first n fibonacci numbers obtained are:\n");
   for(i=1;i<=nfib;i++)
   {
    printf("A[%d] = %d\n",i,A[i]);
   }

   exit(0);
}