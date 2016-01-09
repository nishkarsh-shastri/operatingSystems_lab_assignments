#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <pwd.h>
#include <time.h>
#include <grp.h>

extern int alphasort();

char cwd[1024]; //the global current working directory address
char *tokenList[20];
int numberOfTokens;
char perms_buff[30]; //the permission buffer

//a function to get current working directory
void getCurWorkDir(char* cwd1)
{
	char cwdi[1024];
	if(getcwd(cwdi,sizeof(cwdi))!=NULL) //the getcwd function to get the current working directory
	{
		strcpy(cwd1,cwdi);
	}
	else
	{
		perror("getcwd() error");
	}	
}

void changeCurWorkDir(char *path)
{
	int ret;
	if((ret = chdir(path))==0) //the change directory function
	{
		//printf("Successful Operation\n");
		getCurWorkDir(cwd);

	}
	else
		perror("Bad operation. Failure. No such directory");
}

void printConsole()
{
	printf("%s> ",cwd); //printing the console
}


void makeDirectory(char *dirName) //creating the directory along with proper permissions
{
	int ret;
	ret = mkdir(dirName,S_IRWXU | S_IRWXG | S_IRWXO);		//read,write and search permission for owner, group owner and other users; 
	if(ret==-1)
	{
		perror("Can't create directory");
	}

}

void removeDirectory(char *dirName) //removing a particular directory assuming access rights
{
	int ret;
	ret = rmdir(dirName);
	if(ret==-1)
	{
		perror("Can't remove directory");
	}

}

void getConsoleInput(char *inp1) //getting input from the console
{
	char *p;
	char inpi[3000];

	if(fgets(inpi, sizeof(inpi), stdin)!=NULL)
	{
		if ((p = strchr(inpi, '\n')) != NULL)
      	*p = '\0';
	}
	strcpy(inp1,inpi);
	
}

int file_select(const struct direct *entry) //to avoid the . and .. directory
{
    if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
        return (0);
    else
        return (1);
}
 
int getFileNames() //to get the file names only
{
    int count,i;
    struct direct **files;
 
     
    count = scandir(cwd, &files, file_select, alphasort);
 
    /* If no files found, make a non-selectable menu item */
    if(count <= 0)
      printf("No files in this directory\n");
    else
    {
    	printf("Number of files = %d\n",count);
    	for (i=1; i<count+1; ++i)
        	printf("%s  ",files[i-1]->d_name);
    	printf("\n"); /* flush buffer */
    }
}
  
const char *get_perms(mode_t mode)   //this function will give us all the permissions required
{  
  char ftype = '?';  
  
  if (S_ISREG(mode)) ftype = '-';  
  if (S_ISLNK(mode)) ftype = 'l';  
  if (S_ISDIR(mode)) ftype = 'd';  
  if (S_ISBLK(mode)) ftype = 'b';  
  if (S_ISCHR(mode)) ftype = 'c';  
  if (S_ISFIFO(mode)) ftype = '|';  
  
  sprintf(perms_buff, "%c%c%c%c%c%c%c%c%c%c %c%c%c", ftype,  
  mode & S_IRUSR ? 'r' : '-',  
  mode & S_IWUSR ? 'w' : '-',  
  mode & S_IXUSR ? 'x' : '-',  
  mode & S_IRGRP ? 'r' : '-',  
  mode & S_IWGRP ? 'w' : '-',  
  mode & S_IXGRP ? 'x' : '-',  
  mode & S_IROTH ? 'r' : '-',  
  mode & S_IWOTH ? 'w' : '-',  
  mode & S_IXOTH ? 'x' : '-',  
  mode & S_ISUID ? 'U' : '-',  
  mode & S_ISGID ? 'G' : '-',  
  mode & S_ISVTX ? 'S' : '-');  
  
  return (const char *)perms_buff;  
}  

int getCompletFileDetails() //to get the complete file details
{
	int count,i;
	struct direct **files; //using dir.h library
	struct stat statbuffer;
	char datestring[26];
	struct passwd pwent; 
	struct passwd *pwentp;
	struct group grp;
	struct group* grpt;
	struct tm time; //the time structure for modification date and time
	char buf[1024];
	int total=0;
	count = scandir(cwd, &files, file_select, alphasort); //getting and storing all the available files
	if(count<=0)
	{
		printf("No files in this directory\n");
		return 0;
	}
	printf("Total Number of files in the directory = %d\n",count);

	for(i=0;i<count;i++)//iterate through each file
	{
		if(stat(files[i]->d_name,&statbuffer)==0)//getting the stat of each file
		{
			printf("%10.10s", get_perms(statbuffer.st_mode));  //printing the permission for each file
        	printf(" %d", statbuffer.st_nlink);  				// the number of hard links to the file: the number of directories that has entries for the file
  
	        if (!getpwuid_r(statbuffer.st_uid, &pwent, buf, sizeof(buf), &pwentp))  
	          printf(" %s", pwent.pw_name);  
	        else  
	          printf(" %d", statbuffer.st_uid);  
	  
	        if (!getgrgid_r (statbuffer.st_gid, &grp, buf, sizeof(buf), &grpt))  
	          printf(" %s", grp.gr_name);  
	        else  
	          printf(" %d", statbuffer.st_gid);  
	  
	        /* Print size of file. */  
	        printf(" %5d", (int)statbuffer.st_size);  
	  
	        localtime_r(&statbuffer.st_mtime, &time);  
	        /* Get localized date string. */  
	        strftime(datestring, sizeof(datestring), "%F %T", &time);  
	  
	        printf(" %s %s\n", datestring, files[i]->d_name);  
	        total += statbuffer.st_blocks;//for the blocks allocated for each file

		}
	}
	total/=2;
	printf("Total = %d\n",total); //printing the total blocks allocated in the directory

}

void executeProcess()
{
     pid_t  pid;
     int    status;     
     int flag=0;//the flag for background processing
     if(strcmp(tokenList[numberOfTokens-1],"&")==0)
     {
     	flag = 1;//we have to run the process in background
     	tokenList[numberOfTokens-1] = NULL;
     	numberOfTokens--;
     }
     if ((pid = fork()) < 0) 
     {     
     		/* fork a child process           */
          printf("ERROR: forking child process failed\n");
          return;
     }
     else if (pid == 0) {          /* for the child process:         */
     	
     	if(execvp(tokenList[0],tokenList)<0)
     	{
     		perror("Error executing");
     		exit(1);
     	}  /* execute the command  */

        //The following line is for the execcv function where we have to find the 
     	//path for the execution process. But execvp automatically takes care of that for us.
     	//Uncomment these lines to see the execv function only.

        //if no such path found then search the path      
        /*char* pPath;
       	pPath = getenv ("PATH");  //getting path environment variable

        char tempPath[1000];
        strcpy(tempPath, pPath);
        char *pch;
        pch = strtok(tempPath, ":\r\n");//get tokenized path variables
        while(pch!=NULL)  //using all the paths present in the PATH environment variable
        {
           char path[1000];
           strcpy(path,pch);
           strcat(path, "/");
           strcat(path, tokenList[0]);
          	(path, tokenList);//execute the process
           pch = strtok(NULL, ":\r\n");//no process found
        }*/
        
     }
     else 
     {
         //for the parent: 
     	//Run in foreground
     	if(flag==0)
        	waitpid(pid,&status,0);	
       	else
       	{//run in background
       			return;
       	}
        return;
     }

}
void copyFile(char *fileRead,char *fileWrite)
{
	//Check for the first file if it exists or not
	//We will use the 'access' library function to check the existence and readablity/writabilty of the file

	int rval;
	rval = access(fileRead,F_OK);
	struct stat attrib;
	struct tm *f1time;
	struct tm *f2time;
	FILE *source,*target;
	double seconds;
	char ch;

	if(rval!=0)
	{
		//there is error
		perror("Error with file 1");
		return;
	}
	//else file exists
	//Check for the first file if the read permission is there or not
	rval = access(fileRead,R_OK);
	if(rval!=0)
	{
		//there is error
		perror("Error with file 1, not readable");
		return;
	}
	//Check for the second file if it exist or not

	rval = access(fileWrite,F_OK);
	if(rval==0)
	{
		//the second file exists 
		//Check whether we have the permission to write or not
		rval = access(fileWrite,W_OK);
		if(rval!=0)
		{	
			//there is error
			perror("Error with file 2, not writable");
			return;
		}
		//if permission is there then check the last modification date of the file and compare it with file 1.
		//We wil use the stat function for both the files
		stat(fileRead, &attrib);
		f1time = attrib.st_mtime;
		stat(fileWrite, &attrib);
		f2time = attrib.st_mtime;

		seconds = difftime(mktime(f1time),mktime(f2time));
		if(seconds>0)//File 1 was modified later. We are good to go and copy
		{
			source = fopen(fileRead, "r");
			target = fopen(fileWrite, "w");
			while( ( ch = fgetc(source) ) != EOF )
      			fputc(ch, target);
      		printf("File copied Successfully\n");
      		fclose(source);
      		fclose(target);
      		return;
		}
		else //We have to give error if modification date is recent than file 1
		{
			printf("Error: File 2 is modified later than file 1\n");
			return;
		}		

	}

	//creating the file. Use open function if you want to add permissions also
	target = fopen(fileWrite,"w");//if it does not exist
	//it is assumed that created file will have basic read and write permissions
	source = fopen(fileRead, "r");
	while( ( ch = fgetc(source) ) != EOF )
      			fputc(ch, target);
	printf("File copied Successfully\n");
	fclose(source);
	fclose(target);
	return;

	//create the file at the destined path and then copy the content to that file

}
int main() 
{

	char *inp = (char *)malloc(3000);//the input string
   	getCurWorkDir(cwd);
   	int i;
  	char *result;
  	char *temp1;
  	char *temp2;

  	while(1)
  	{
  		numberOfTokens = 0;
  		printConsole();
  		char *inp = (char *)malloc(3000);
  		getConsoleInput(inp); 
		fflush(stdin);
  		while((tokenList[numberOfTokens] = strsep(&inp," \t\n")) != NULL) 
  		{
			if(strcmp(tokenList[numberOfTokens], "")) 
				numberOfTokens++;
		}	//tokenize the input based on spaces separated strings	
		free(inp);

		if(tokenList[0]!=NULL)//we have given some input
		{		
			if(strcmp(tokenList[0],"pwd")==0)//present working directory
			{
				printf("%s\n",cwd);
			}
			else if(strcmp(tokenList[0],"cd")==0)//change directory
			{
				temp1 = tokenList[1];
				if(numberOfTokens == 2)
				{
					changeCurWorkDir(temp1);
				}
			}
			else if(strcmp(tokenList[0],"mkdir")==0)//make a directory
			{
				temp1 = tokenList[1];
				if(numberOfTokens == 2)
				{
					printf("Execution\n");
					makeDirectory(temp1);
				}
			}
			else if(strcmp(tokenList[0],"rmdir")==0)//remove a directory
			{
				temp1 = tokenList[1];
				if(numberOfTokens == 2)
				{
					printf("removal\n");
					removeDirectory(temp1);
				}
			}
			else if(strcmp(tokenList[0],"ls")==0)//list the files
			{
				if(numberOfTokens == 1)
					getFileNames();
				else if(numberOfTokens==2 && strcmp(tokenList[1],"-l")==0) //list the detailed file address
				{
					getCompletFileDetails();
				}
			}
			else if(strcmp(tokenList[0],"cp")==0)//file copying
			{
				if(numberOfTokens==3)
				{
					copyFile(tokenList[1],tokenList[2]);
				}
				else
				{
					printf("Invalid number Of strings\n");
				}
			}
			else if(strcmp(tokenList[0],"exit")==0)//exit
			{
				return 0;
			}
			else if(tokenList!=NULL)//the process needs to be executed. THe first argument must have been the process name followed by all the arguments.
			{
				executeProcess();
			}
		}
		//no input given, try again
  	}
}