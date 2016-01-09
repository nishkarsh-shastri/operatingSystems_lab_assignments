#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

char cwd[1024]; //the global current working directory address

//a function to get current working directory
void getCurWorkDir(char* cwd1)
{
	char cwdi[1024];
	if(getcwd(cwdi,sizeof(cwdi))!=NULL)
	{
		strcpy(cwd1,cwdi);
	}
	else
	{
		perror("getcwd() error");
	}
	int i = 0;
	
}

void changeCurWorkDir(char *path)
{
	int ret;
	if((ret = chdir(path))==0)
	{
		printf("Successful Operation\n");
		getCurWorkDir(cwd);

	}
	else
		printf("Bad operation. Failure. No such directory\n");
}

void printConsole()
{
	printf("%s >",cwd);
}

void getConsoleInput(char *inp1)
{
	char *p;
	char inp[3000];
	if(fgets(inp, sizeof(inp), stdin)!=NULL)
	{
		if ((p = strchr(inp, '\n')) != NULL)
      	*p = '\0';
	}
	strcpy(inp1,inp);
}

int main() 
{

   	getCurWorkDir(cwd);
  	printConsole();
  	char *s = "../../Coding Stuffs";
   	changeCurWorkDir(s);
	char inp[1024];
	char *p;
	while(strcmp(inp,"-23")!=0)
	{
		printConsole();
		getConsoleInput(inp);
		printf("%s\n",inp);
		
	}
   return 0;
}