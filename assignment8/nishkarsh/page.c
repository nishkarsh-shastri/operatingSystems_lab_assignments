#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>


int n,m,d;

/* 
n <-- Represents the order of matrix
d <-- Represents the number of integers that can be stored in a page -- Used only during matrix and array creation
m <-- Number of pages in a page table
*/

//function for FIFO -- Will use queue implemented using array limits
int FIFO(int *pageArr,int len)
{
	char filePrint[100];
	sprintf(filePrint,"n = %d\td = %d\nm\tfaults\n",n,d);
	//now create files and add the above values in each of the files
	int fd = open("./fifo.dat",O_WRONLY | O_CREAT | O_TRUNC,0666);
	write(fd,filePrint,strlen(filePrint));
	printf("What is the Base page size from where we will be incrementing by one in each iteration\n");
	scanf("%d",&m);
	printf("How many iterations do you want to do?\n");
	int num_iter;
	scanf("%d",&num_iter);
	int i,j,k;
	
	//we will use sliding window method to remember the queue positions
	for(k=0;k<num_iter;k++)
	{
		printf("Iter = %d , m(PageTable Size) = %d\n",k+1,m);
		int *pageMem = (int*)malloc(len*sizeof(int));
		//queue using array implementation
		int start = 0;
		int end = 0;
		int fault = 0;
		for(i=0;i<len;i++)
		{
			int flag = 0;

			for(j=start;j<end;j++)
			{
				if(pageArr[i]==pageMem[j])
				{
					flag = 1;//no page fault
					break;
				}
			}
			if(flag==0)
			{
				if((end-start)==m)
				{
					printf("Fault found at %d\n",pageArr[i]);
					fault++;
					start++; //removed the firt in page 
					pageMem[end]=pageArr[i];// added the new value to the queue
					end++;
				}
				else
				{
					//blank slot found
					printf("%d is inserted in the page table\n",pageArr[i]);
					fault++;
					pageMem[end]=pageArr[i]; //add the new value to the queue
					end++;
				}
			}
		}
		printf("Number of faults for this iteration of fifo = %d\n",fault);
		sprintf(filePrint,"%d\t%d\n",m,fault);
		write(fd,filePrint,strlen(filePrint));
		free(pageMem);
		m = m+1;
	}

	close(fd);
	return 0;

}
//function for LFU  -- Will use frequency,time when it was last used  as deciding factors and then ties will be resolved by FIFO
int LFU(int *pageArr,int len)
{
	char filePrint[100];
	sprintf(filePrint,"n = %d\td = %d\nm\tfaults\n",n,d);
	//now create files and add the above values in each of the files
	int fd = open("./lfu.dat",O_WRONLY | O_CREAT | O_TRUNC,0666);
	write(fd,filePrint,strlen(filePrint));
	printf("What is the Base page size from where we will be incrementing by one in each iteration\n");
	scanf("%d",&m);
	printf("How many iterations do you want to do?\n");
	int num_iter;
	scanf("%d",&num_iter);
	int i,j,k;

	for(k=0;k<num_iter;k++)
	{
		printf("Iter = %d , m(PageTable Size) = %d\n",k+1,m);
		int fault=0,timer=1,start=0,end=0;//inititalize values
		int *pageMem = (int*)malloc(m*sizeof(int)); //page Memory
		int *count = (int*)malloc(m*sizeof(int)); //frequency of each entry
		int *timeseries = (int*)malloc(m*sizeof(int));	//timer for each entry
		int replace; //the replaced page

		for(i=0;i<len;i++)
		{
			int flag=0;
			for(j=start;j<end;j++)
			{
				if(pageArr[i]==pageMem[j])
				{
					//no page fault
					flag =1;
					//update frequency
					count[j]++;  //update the count
					//update timer
					timeseries[j] = timer++; //update the time value for that entry
					break;
				}
			}
			if(flag==0)//the page fault condition
			{
				if((end-start)==m)
				{
					printf("Fault found at %d\n",pageArr[i]);
					fault++;
					replace = start;
					for(j=start;j<end;j++)
					{
						if(count[j]<=count[replace])
							if(count[j]<count[replace] || timeseries[j]< timeseries[replace]) //compare frequency and timer
							{
								replace = j;
							}
					}
					pageMem[replace] = pageArr[i];
					count[replace] = 1;
					timeseries[replace]=timer++;
				}
				else
				{
					printf("Inserting %d \n",pageArr[i]);
					fault++;
					pageMem[end] = pageArr[i];
					count[end] = 1;
					timeseries[end] = timer++;
					end++;
				}
			}
		}

		printf("Number of faults for this iteration of lfu = %d\n",fault);
		sprintf(filePrint,"%d\t%d\n",m,fault);
		write(fd,filePrint,strlen(filePrint));
		free(pageMem);
		free(count);
		free(timeseries);
		m = m+1;
	}
	close(fd);
	return 0;
}
//function for LRU  -- Will use Stack implemented using arrays and sliding window of size = pageTableSize


int LRU(int *pageArr,int len) //only timer is used compared to the above
{
	char filePrint[100];
	sprintf(filePrint,"n = %d\td = %d\nm\tfaults\n",n,d);
	//now create files and add the above values in each of the files
	
	int fd = open("./lru.dat",O_WRONLY | O_CREAT | O_TRUNC,0666);
	write(fd,filePrint,strlen(filePrint));
	printf("What is the Base page size from where we will be incrementing by one in each iteration\n");
	scanf("%d",&m);
	printf("How many iterations do you want to do?\n");
	int num_iter;
	scanf("%d",&num_iter);
	int i,j,k;

	for(k=0;k<num_iter;k++)
	{
		printf("Iter = %d , m(PageTable Size) = %d\n",k+1,m);
		int fault=0,timer=1,start=0,end=0;
		int *pageMem = (int*)malloc(m*sizeof(int)); //page Memory
		int *timeseries = (int*)malloc(m*sizeof(int));	//timer for each entry
		int replace; //the replaced page

		for(i=0;i<len;i++)
		{
			int flag=0;
			for(j=start;j<end;j++)
			{
				if(pageArr[i]==pageMem[j])
				{
					//no page fault
					flag =1;
					//update timer
					timeseries[j] = timer++;
					break;
				}
			}
			if(flag==0)//the page fault condition
			{
				if((end-start)==m)
				{
					printf("Fault found at %d\n",pageArr[i]);
					fault++;
					replace = start;
					for(j=start;j<end;j++)
					{
						if(timeseries[j]< timeseries[replace])
						{
							replace = j;
						}
					}
					pageMem[replace] = pageArr[i];
					timeseries[replace]=timer++;
				}
				else
				{
					printf("Inserting %d \n",pageArr[i]);
					fault++;
					pageMem[end] = pageArr[i];
					timeseries[end] = timer++;
					end++;
				}
			}
		}

		printf("Number of faults for this iteration of lfu = %d\n",fault);
		sprintf(filePrint,"%d\t%d\n",m,fault);
		write(fd,filePrint,strlen(filePrint));
		free(pageMem);
		free(timeseries);
		m = m+1;
	}
	close(fd);
	return 0;
}
//function for SCA  -- Will use reference bits to handle the page faults
int SCA(int *pageArr,int len)
{
	char filePrint[100];
	sprintf(filePrint,"n = %d\td = %d\nm\tfaults\n",n,d);
	//now create files and add the above values in each of the files
	int fd = open("./sca.dat",O_WRONLY | O_CREAT | O_TRUNC,0666);
	write(fd,filePrint,strlen(filePrint));

	printf("What is the Base page size from where we will be incrementing by one in each iteration\n");
	scanf("%d",&m);
	printf("How many iterations do you want to do?\n");
	int num_iter;
	scanf("%d",&num_iter);
	int i,j,k;

	for(k=0;k<num_iter;k++)
	{
		printf("Iter = %d , m(PageTable Size) = %d\n",k+1,m);
		int fault=0,start=0,end=0;
		int *pageMem = (int*)malloc(m*sizeof(int)); //page Memory
		int *reference = (int*)malloc(m*sizeof(int)); //frequency of each entry
		int last = 0; //the previous page found
		int replace;

		for(i=0;i<len;i++)
		{
			int flag=0;
			for(j=start;j<end;j++)
			{
				if(pageArr[i]==pageMem[j])
				{
					//no page fault
					flag = 1;
					reference[j] = 1;
					break;
				}
			}
			if(flag==0)//the page fault condition
			{
				if((end-start)==m)
				{
					printf("Fault found at %d\n",pageArr[i]);
					fault++;
					replace = -1;
					for(j=last;j<end;j++)
					{
						if(reference[j]==0)
						{
							replace=j;
							last = replace+1;
							break;
						}
						else
						{
							reference[j] = 0;
						}
					}
					if(replace==-1)
					{
						//no page with reference bit 0 found
						for(j=start;j<=last;j++)
						{
							if(reference[j] == 0)
							{
								replace = j;
								last = replace + 1;
								break;
							}
							else
							{
								reference[j] = 0;
							}
						}

					}
					if(last == end)
					{
						last = 0;
					}
					pageMem[replace] = pageArr[i];
					reference[replace] = 0;
				}
				else
				{
					printf("Inserting %d \n",pageArr[i]);
					fault++;
					pageMem[end] = pageArr[i];
					reference[end] = 0;
					end++;
				}
			}
		}

		printf("Number of faults for this iteration of lfu = %d\n",fault);
		sprintf(filePrint,"%d\t%d\n",m,fault);
		write(fd,filePrint,strlen(filePrint));
		free(pageMem);
		free(reference);
		m = m+1;
	}
	close(fd);
	return 0;
}





int main()
{
	int i,j,k;
	printf("Enter the size(order) of the matrix for multiplication:\n");
	scanf("%d",&n);
	printf("Enter the size of pages (no of possible pages in page table)\n");
	scanf("%d",&d);

	//Generate the matrix
	// len -- Size of page array required for multiplication
	int len = n*n*n*2 + n*n;
	int *pageArr = (int*)malloc(len*sizeof(int));
	//Matrix 1
	int **mat_1 = (int**)malloc(n*sizeof(int*));
	//Matrix 2
	int **mat_2 = (int**)malloc(n*sizeof(int*));
	//Matrix 3 - output matrix
	int **output = (int**)malloc(n*sizeof(int*));

	for(i=0;i<n;i++)
	{
		mat_1[i] = (int *)malloc(n*sizeof(int));
		mat_2[i] = (int *)malloc(n*sizeof(int));
		output[i] = (int *)malloc(n*sizeof(int));
	}
	//time to fill the matrices

	//first matrix is supposed to start from 101,second from 501 and output from 1001
	int base_1 = 101;
	int base_2 = 501;
	int base_3 = 1001;
	int repeated = 0;
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			mat_1[i][j]=base_1;
			mat_2[i][j]=base_2;
			output[i][j]=base_3;
			repeated++;
			if(repeated==d)
			{
				repeated = 0;
				base_1++;
				base_2++;
				base_3++;
			}
		}
	}
	//matrices are filled completely, lets generate the multiplication string
	//fill them in pageArr
	repeated = 0;
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			for(k=0;k<n;k++)
			{
				pageArr[repeated++]=mat_1[i][k];
				pageArr[repeated++]=mat_2[k][j];
			}
			pageArr[repeated++]=output[i][j];
		}
	}
	printf("Here\n");
	//lets print the page array
	for(i=0;i<repeated;i++)
	{
		printf("%d  ",pageArr[i]);
	}
	printf("\n");

	int choice;
	printf("\n\n************************************************************************\n\n");
	printf("What kind of algorithm do you want??Choose one of the following\n");
	printf("1) FIFO - First In First Out\n");
	printf("2) LRU - Least Recently Used\n");
	printf("3) LFU - Least Frequently Used\n");
	printf("4) SCA - Second Chance Algorithm\n");
	printf("5) Done and exit\n");
	while(1)
	{
		printf("Enter your choice\n");
		scanf("%d",&choice);
		switch(choice)
		{
			case 1:
					FIFO(pageArr,repeated);
					break;
			case 2:
					LRU(pageArr,repeated);
					break;
			case 3:
					LFU(pageArr,repeated);
					break;
			case 4:
					SCA(pageArr,repeated);
					break;
			case 5:
					exit(0);
			default:
					printf("Incorrect Choice, Your choices are\n");
					printf("1) FIFO - First In First Out\n");
					printf("2) LRU - Least Recently Used\n");
					printf("3) LFU - Least Frequently Used\n");
					printf("4) SCA - Second Chance Algorithm\n");
					printf("5) Done and exit\n");
					break;
		}
	}

	return 0;
}