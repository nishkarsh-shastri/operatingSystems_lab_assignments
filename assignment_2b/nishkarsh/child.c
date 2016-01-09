/* The Library headers for implementing functions */
#include <stdio.h> /*typically in /usr/include*/
#include <stdlib.h>
#include <string.h>
#include <time.h>

//The Macros defined for interprocess communication
#define REQUEST 100
#define PIVOT 200
#define LARGE 300
#define SMALL 400
#define READY 500
#define EXIT 600


int main(int argc,char *argv[])
{
    srand((unsigned int) time(NULL));//randomize seed

    int child_read = atoi(argv[1]);//get the child read pipe as argument
    int child_write = atoi(argv[2]);//get the child write pipe as argument

    int len,val,send,pivot,count;//variables for communication
    pid_t pid;
    char fileName[100];//the file used for reading the numbers
    int id; //the procss id
    int i;//used for iteration purposes
    FILE *f;//the file pointer
    int a[5];//the integer storage
    int arrSize = 5;//used for array size

    


    while(1)
    {
        len = read(child_read,&val,sizeof(val));//get the id of the process
        if(len<0)
        {
            perror("Child: Failed to read data from pipe");
            exit(EXIT_FAILURE);
        }
        else
        {
            id = val+1;
            sprintf(fileName,"data%d.txt",id);//get filename containing the integers
            f = fopen(fileName,"r");
            if(f==NULL)
            perror("ERROR");
            for(i=0;i<5;i++)
            {
                fscanf(f,"%d ",&a[i]);//read the integers
            }
            fclose(f);//close file
            send = READY;
            printf("----- Child %d sends READY\n",id);//child is ready
            write(child_write, &send, sizeof(send));//send ready signal

            while(1)//wait for command
            {

                len = read(child_read,&val,sizeof(val));//get the command
                if(len<0)
                {
                    perror("Child: Failed to read data from pipe");
                    exit(EXIT_FAILURE);
                }
                switch(val)//now do operation based on integer received from parent
                {
                    case REQUEST://get the request to send a random number to parent
                        if(arrSize==0)//no element present
                        {
                            printf("----- Child %d sends -1 to parent\n",id);
                            send = -1;
                            write(child_write, &send, sizeof(send));
                            break;
                        }
                        else
                        {
                            send = -1;
                            while(send==-1)//get the positive non-deleted number
                            {
                                i = rand()%5;
                                send = a[i];
                            }
                            printf("----- Child %d sends %d to parent\n",id,send);
                            write(child_write, &send, sizeof(send));  //send the number via child-parent pipe
                            break;                      
                        }
                        
                    case PIVOT: //get the pivot command
                        pivot = -1;
                        while(pivot==-1)//wait till it get the pivot element
                        len = read(child_read,&pivot,sizeof(pivot));
                        count = 0;
                        for(i=0;i<5;i++)//report the non-deleted number greater than pivot
                        {
                            if(a[i]!=-1 && a[i]>pivot)
                            {
                                count++;
                            }
                        }
                        send = count;
                        printf("----- Child %d receives pivot and replies %d\n",id,send);
                        write(child_write, &send, sizeof(send));  //send count to the parent
                        break;

                    case SMALL:
                        pivot = -1;
                        while(pivot==-1)//get the pivot again
                        len = read(child_read,&pivot,sizeof(pivot));
                        count = 0;
                        //remove those which are less than pivot
                        for(i=0;i<5;i++)
                        {
                            if(a[i]!=-1 && a[i]<pivot)
                            {
                                a[i]=-1;
                                arrSize--;
                                count++;
                            }
                        }
                        send = count;
                        printf("----- Child %d removing %d elements smaller than %d\n",id,count,pivot);
                        //send acknowledgement back
                        write(child_write, &send, sizeof(send));
                        break;
                    case LARGE:
                        pivot = -1;
                        while(pivot==-1)
                        len = read(child_read,&pivot,sizeof(pivot));//get pivot again
                        count = 0;
                        //remove those element greater than pivot
                        for(i=0;i<5;i++)
                        {
                            if(a[i]!=-1 && a[i]>pivot)
                            {
                                a[i]=-1;
                                arrSize--;
                                count++;
                            }
                        }
                        send = count;
                        printf("----- Child %d removing %d elements greater than %d\n",id,count,pivot);
                        //send acknowledgement
                        write(child_write, &send, sizeof(send));
                        break;

                   case EXIT:
                        printf("----- Child %d terminates\n",id);
                        //Exit from the process. All work done.
                        close(child_read);
                        close(child_write);
                        return EXIT_SUCCESS;    

                }
                
            }
        }
    }
}