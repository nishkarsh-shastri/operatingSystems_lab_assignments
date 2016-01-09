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
#include <fcntl.h>

extern int alphasort();

char cwd[1024]; //the global current working directory address
char *tokenList[50];
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
 
void getFileNames() //to get the file names only
{
    int count, i, j, len;
    struct dirent **files;
 
     
    count = scandir(cwd, &files, file_select, alphasort);
 
    /* If no files found, make a non-selectable menu item */
    if(count <= 0)
      printf("No files in this directory\n");
    else
    {
    	printf("Number of files = %d\n",count);
    	for (i = 0; i < count; ++i) {
    		len = strlen(files[i]->d_name);
    		printf("%s", files[i]->d_name);
    		for(j = 0; j < 30 - len; ++j)
    			printf(" ");
    		if((i + 1)%5 == 0)printf("\n");
    	}
        	
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

void getCompletFileDetails() //to get the complete file details
{
	int count,i;
	struct dirent **files; //using dir.h library
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
	if(count <= 0)
	{
		printf("No files in this directory\n");
	}
	printf("Total Number of files in the directory = %d\n",count);

	for(i=0;i<count;i++)//iterate through each file
	{
		if(stat(files[i]->d_name,&statbuffer)==0)//getting the stat of each file
		{
			printf("%10.10s", get_perms(statbuffer.st_mode));  //printing the permission for each file
        	printf(" %d", statbuffer.st_nlink);  				// the number of hard links to the file: 
        														// the number of directories that has entries for the file
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
     int    status, flag = 0, outflag = 0, inflag = 0, i, j;
     char *outfile, *infile;
     int fdwrite, fdread;

     for (i = 0; i < numberOfTokens; ++i) {
     	if(tokenList[i][0] == '>') {
     		outfile = strdup(tokenList[i] + 1);
     		for (j = i + 1; j < numberOfTokens; ++j)
     			strcpy(tokenList[j-1], tokenList[j]);
     		numberOfTokens -= 1;
     		if((fdwrite = open(outfile, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
     			perror("Could not open file for writing");
     			return;
     		}
     		outflag = 1;
     		break;
     	}
     	if(!strcmp(tokenList[i], ">")) {
     		outfile = strdup(tokenList[i + 1]);
     		for (j = i + 2; j < numberOfTokens; ++j)
     			strcpy(tokenList[j-2], tokenList[j]);
     		numberOfTokens -= 2;
     		if((fdwrite = open(outfile, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
     			perror("Could not open file for writing");
     			return;
     		}
     		outflag = 1;
     		break;
     	}
     }

     for (i = 0; i < numberOfTokens; ++i) {
     	if(tokenList[i][0] == '<') {
     		infile = strdup(tokenList[i] + 1);
     		for (j = i + 1; j < numberOfTokens; ++j)
     			strcpy(tokenList[j-1], tokenList[j]);
     		numberOfTokens -= 1;
     		if((fdread = open(infile, O_RDONLY)) == -1) {
     			perror("Could not open file for reading");
     			return;
     		}
     		inflag = 1;
     		break;
     	}
     	if(!strcmp(tokenList[i], "<")) {
     		outfile = strdup(tokenList[i + 1]);
     		for (j = i + 2; j < numberOfTokens; ++j)
     			strcpy(tokenList[j-2], tokenList[j]);
     		numberOfTokens -= 2;
     		if((fdread = open(infile, O_RDONLY)) == -1) {
     			perror("Could not open file for reading");
     			return;
     		}
     		inflag = 1;
     		break;
     	}
     }

     if(strcmp(tokenList[numberOfTokens - 1], "&")==0)
     {
     	flag = 1;//we have to run the process in background
     	tokenList[numberOfTokens-1] = NULL;
     	numberOfTokens--;
     }
     tokenList[numberOfTokens] = NULL;
     if ((pid = fork()) < 0) 
     {     
     	/* fork a child process */
        printf("ERROR: forking child process failed\n");
        return;
     }
     if (pid == 0) {          
     /* for the child process: */
     	if (outflag) dup2(fdwrite, STDOUT_FILENO);
     	if (inflag) dup2(fdread, STDIN_FILENO);
     	if(execvp(tokenList[0],tokenList)<0)
     	{
     		perror("Error executing");
     		exit(1);
     	}  
     }
     else {
        //for the parent: 
     	//Run in foreground
     	if(flag==0)
        	waitpid(pid,&status,0);	
       	else
       	{
       		//run in background
       		return;
       	}
        return;
     }
}

void executeProcess_piped() {
	char *paramlist[10], *tokenListNew[50], *part, *token;
	int i = 0, j = 0, k, pipes = 0, (*fd)[2];
	pid_t* childarray;
	while(i < numberOfTokens) {
		if(strchr(tokenList[i],'|') && strcmp(tokenList[i], "|")) {
			token = strdup(tokenList[i]);
			while((part = strsep(&token, "|"))) {
				if(strcmp(part,"")) tokenListNew[j++] = strdup(part);
				tokenListNew[j++] = strdup("|");	
				pipes++;	
			}
			--j;
			--pipes;
		}
		else {
			tokenListNew[j++] = strdup(tokenList[i]);
			if(!strcmp(tokenList[i],"|"))pipes++;			
		}
		++i;
	}
	numberOfTokens = j;

	fd = (int(*)[2])malloc(pipes * sizeof(int[2]));
	childarray = (pid_t*)malloc((pipes + 1) * sizeof(pid_t));
	memset(childarray, -1, sizeof(pid_t) * (pipes + 1));
	for (i = 0; i < pipes; ++i) {
		if (pipe(fd[i])) {
			perror("Could not create some pipe");
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < pipes + 1; ++i) {
		if(!i || childarray[0] > 0) {
			childarray[i] = fork();
			if (childarray[i] < 0) {
				perror("Unable to fork some child");
				exit(EXIT_FAILURE);
			}
			if (childarray[i] == 0) {
				for (j = 0; j < i; ++j) childarray[j] = -1;
			}
		}
	}

	j = 0;
	for (i = 0; i < pipes + 1; ++i) {
		k = 0;
		for (;j < numberOfTokens && strcmp(tokenListNew[j],"|"); ++j, ++k) 
			paramlist[k] = strdup(tokenListNew[j]);
		j++;
		paramlist[k] = 0;
		if (childarray[i] == 0) {
			if(!i) {
				dup2(fd[i][1], STDOUT_FILENO);
				execvp(paramlist[0],paramlist);
			}
			else if(i < pipes) {
				dup2(fd[i-1][0], STDIN_FILENO);
				dup2(fd[i][1], STDOUT_FILENO);
				execvp(paramlist[0], paramlist);
			}
			else {
				dup2(fd[i-1][0], STDIN_FILENO);
				execvp(paramlist[0], paramlist);
			}
		}
	}

	for (i = 0; i < pipes + 1; ++i) {
		wait(0);
	}
}

void copyFile(char *fileRead,char *fileWrite) {
	//Check for the first file if it exists or not
	//We will use the 'access' library function to check the existence and readablity/writabilty of the file

	int rval;
	rval = access(fileRead,F_OK);
	struct stat attribread;
	struct stat attribwrite;
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
		stat(fileRead, &attribread);
		//f1time = localtime(&(attribread.st_mtime));
		stat(fileWrite, &attribwrite);
		//f2time = localtime(&(attribwrite.st_mtime));

		//seconds = difftime(mktime(f1time),mktime(f2time));
		seconds = difftime(attribread.st_mtime, attribwrite.st_mtime);
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
   	int i, flag;
  	char *result;
  	char *temp1;
  	char *temp2;

  	while(1)
  	{
  		numberOfTokens = 0;
  		printConsole();
  		char *inp = (char *)malloc(3000);
  		getConsoleInput(inp); 
  		if (strchr(inp,'|')) flag = 1;
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
				if (flag)
					executeProcess_piped();
				else executeProcess();
			}
		}
		//no input given, try again
  	}
}