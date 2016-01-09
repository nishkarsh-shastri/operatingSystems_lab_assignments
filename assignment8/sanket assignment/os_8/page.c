#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


void fifo(int* arr, int len);
void lfu(int* arr, int len);
void lru(int* arr, int len);
void sca(int* arr, int len);

int m;

int main(int argc, char const *argv[])
{
	
	/** Variable Declaration **/
	int n,d;
	int choice;
	// int arr[] = {1,2,3,4,1,2,5,1,2,3,4,5};
	// int len = 12;

	printf("Enter n (order of matrix): ");
	scanf("%d",&n);
	printf("Enter d (no of int in a page): ");
	scanf("%d",&d);
	// printf("Enter m (no of frames): ");
	// scanf("%d",&m);

	/** Generating reference string in an array **/
	int *arr = (int*)malloc(sizeof(int)*(n*n*n*2 + n*n));
	int len = n*n*n*2 + n*n;

	int **a = (int**)malloc(sizeof(int*)*n);
	int **b = (int**)malloc(sizeof(int*)*n);
	int **r = (int**)malloc(sizeof(int*)*n);
	int i,j,k;
	for(i=0;i<n;i++)
	{
		a[i] = (int*)malloc(sizeof(int)*n);
		b[i] = (int*)malloc(sizeof(int)*n);
		r[i] = (int*)malloc(sizeof(int)*n);
	}
	int count = d;
	int ia = 101;
	int ib = 501;
	int ir = 1001;
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			a[i][j] = ia;
			b[i][j] = ib;
			r[i][j] = ir;
			count--;
			if(count == 0)
			{
				count = d;
				ia++;
				ib++;
				ir++;
			}
		}
	}
	
	int l = 0;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			for (k = 0; k < n; k++)
			{
				
				arr[l++] = a[i][k];
				arr[l++] = b[k][j];
				// sum = sum + a[i][k]*b[k][j];
			}

			arr[l++] = r[i][j];
			// r[i][j] = sum;
			// sum = 0;
		}
	}

	for(i=0;i<l;i++)
		printf("%d  ",arr[i]);

	char temp[50];
	int fd;
	sprintf(temp, "n = %d\nd = %d\nm\tfault\n",n,d);
	fd = open("./fifo.txt",O_RDWR | O_CREAT ,0777);
	write(fd,temp,strlen(temp));
	close(fd);
	fd = open("./lfu.txt",O_RDWR | O_CREAT ,0777);
	write(fd,temp,strlen(temp));
	close(fd);
	fd = open("./lru.txt",O_RDWR | O_CREAT ,0777);
	write(fd,temp,strlen(temp));
	close(fd);
	fd = open("./sca.txt",O_RDWR | O_CREAT ,0777);
	write(fd,temp,strlen(temp));
	close(fd);

	while(1){
		printf("\n\n------------------------------------------------\n");
		printf("Choose one of the following options\npress 1: FIFO\npress 2: LFU\npress 3: LRU\npress 4: Second Chance Algorithm\npress 0: Exit\n");
		scanf("%d",&choice);
		switch(choice)
		{
			case 1:
					fifo(arr,len);
					break;
			case 2:
					lfu(arr,len);
					break;
			case 3:
					lru(arr,len);
					break;
			case 4:
					sca(arr,len);
					break;					
			case 0:
					exit(0);
			default:
					printf("Incorrect Choice, try again...\n");
					sleep(1);
					break;

		}
	}
	
	return 0;
}

void fifo(int* arr, int len)
{
	int fd = open("./fifo.txt",O_RDWR | O_CREAT | O_APPEND,0777);
	printf("Enter m : ");
	scanf("%d",&m);
	// printf("Inside fifo\n");	
	int *mem = (int*)malloc(sizeof(int)*len);
	int start = 0;
	int end = 0;
	int fault = 0;
	int i,j;

	start = 0;
	end = 0;
	
	for(i=0;i<len;i++)
	{
		int flag = 0;
		for(j=start;j<end;j++)
		{
			if(arr[i] == mem[j])
			{
				flag = 1;
				break;
			}

		}
		if(flag == 0 && (end-start) == m)
		{
			//page fault
			printf("Fault at %d\n",arr[i]);
			fault += 1;
			start += 1;
			mem[end] = arr[i];
			end += 1;
		}
		else if(flag == 0)
		{
			printf("%d inserted\n",arr[i]);
			fault += 1;
			mem[end] = arr[i];
			end += 1;
		}
	}
	printf("No of faults in fifo is %d\n",fault);	
	char temp[100];
	sprintf(temp,"%2d\t%2d\n",m,fault);
	write(fd, temp, strlen(temp));
	close(fd);
	free(mem);
}

void lfu(int* arr, int len)
{
	int fd = open("./lfu.txt",O_RDWR | O_CREAT | O_APPEND,0777);
	printf("Enter m : ");
	scanf("%d",&m);
	/** Tie resolved by FIFO **/
	// printf("Inside lfu\n");
	int fault = 0,i,j;
	int *mem = (int*)malloc(sizeof(int)*m);
	int *count = (int*)malloc(sizeof(int)*m);
	int *ts = (int*)malloc(sizeof(int)*m);
	int timer = 1;
	int start = 0, end = 0;
	int replace;

	for(i=0;i<len;i++)
	{
		int flag = 0;
		for(j=start;j<end;j++)
		{
			if(arr[i] == mem[j])
			{
				flag = 1;
				count[j]++;
				ts[j] = timer++;
				break;
			}

		}
		if(flag == 0 && (end-start) == m)
		{
			//page fault
			printf("Fault at %d\n",arr[i]);
			fault += 1;
			replace = start;
			for(j=start;j<end;j++)
			{
				if(count[j] <= count[replace])
				{
					if(count[j] < count[replace] || ts[j] < ts[replace])
						replace = j;
				}
			}
			mem[replace] = arr[i];
			count[replace] = 1;
			ts[replace] = timer++;
			
		}
		else if(flag == 0)
		{
			printf("%d inserted\n",arr[i]);
			fault += 1;
			mem[end] = arr[i];
			count[end] = 1;
			ts[end] = timer++;
			end += 1;
		}
	}
	printf("No of faults in lfu is %d\n",fault);
	char temp[100];
	sprintf(temp,"%2d\t%2d\n",m,fault);
	write(fd, temp, strlen(temp));
	close(fd);
	free(mem);
	free(count);
	free(ts);
	/** Tie is broken by FIFO **/
}

void lru(int* arr, int len)
{
	int fd = open("./lru.txt",O_RDWR | O_CREAT | O_APPEND,0777);
	// printf("Inside lru\n");
	printf("Enter m : ");
	scanf("%d",&m);
	int fault = 0,i,j;
	int timer = 1;
	int *mem = (int*)malloc(sizeof(int)*m);
	int *ts = (int*)malloc(sizeof(int)*m);
	int start = 0, end = 0;
	int replace;

	for(i=0;i<len;i++)
	{
		int flag = 0;
		for(j=start;j<end;j++)
		{
			if(arr[i] == mem[j])
			{
				flag = 1;
				ts[j] = timer++;
				break;
			}

		}
		if(flag == 0 && (end-start) == m)
		{
			//page fault
			printf("Fault at %d\n",arr[i]);
			fault += 1;
			replace = start;
			for(j=start;j<end;j++)
			{
				if(ts[j] < ts[replace])
				{
					replace = j;
				}
			}
			mem[replace] = arr[i];
			ts[replace] = timer++;
			
		}
		else if(flag == 0)
		{
			printf("%d inserted\n",arr[i]);
			fault += 1;
			mem[end] = arr[i];
			ts[end] = timer++;
			end += 1;
		}
	}
	printf("No of faults in lru is %d\n",fault);
	char temp[100];
	sprintf(temp,"%2d\t%2d\n",m,fault);
	write(fd, temp, strlen(temp));
	close(fd);
	free(mem);
	free(ts);
}

void sca(int* arr, int len)
{
	/** Tie is broken by FIFO **/
	int fd = open("./sca.txt",O_RDWR | O_CREAT | O_APPEND,0777);
	printf("Enter m : ");
	scanf("%d",&m);
	int *mem = (int*)malloc(sizeof(int)*m);
	int *ref = (int*)malloc(sizeof(int)*m);
	int start = 0;
	int end = 0;
	int fault = 0;
	int i,j;
	int prev = 0;

	start = 0;
	end = 0;
	
	for(i=0;i<len;i++)
	{
		int flag = 0;
		for(j=start;j<end;j++)
		{
			if(arr[i] == mem[j])
			{
				flag = 1;
				ref[j] = 1;
				break;
			}

		}
		if(flag == 0 && (end-start) == m)
		{
			//page fault
			printf("Fault at %d\n",arr[i]);

			int replace = -1;
			fault += 1;
			
			
			for(j=prev;j<end;j++)
			{
				if(ref[j] == 0)
				{
					replace = j;
					prev = replace + 1;
					break;
				}
				else
				{
					ref[j] = 0;
				}
			}

			if(replace == -1)
			{
				for(j=start;j<=prev;j++)
				{
					if(ref[j] == 0)
					{
						replace = j;
						prev = replace + 1;
						break;
					}
					else
					{
						ref[j] = 0;
					}
				}
			}
			if(prev == end) prev = 0;
			mem[replace] = arr[i];
			ref[replace] = 0;
		
		}
		else if(flag == 0)
		{
			printf("%d inserted\n",arr[i]);
			fault += 1;
			mem[end] = arr[i];
			ref[end] = 0;
			end += 1;
		}
	}
	printf("No of faults in sca is %d\n",fault);
	char temp[100];
	sprintf(temp,"%2d\t%2d\n",m,fault);
	write(fd, temp, strlen(temp));
	close(fd);
	free(mem);
	free(ref);
}