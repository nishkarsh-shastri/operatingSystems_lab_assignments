#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define SIZE 80
#define CHILDS 5
#define READY 0
#define REQUEST 1
#define PIVOT 2
#define SMALL 3
#define LARGE 4
#define SHOW 10


int main(int argc, char const *argv[])
{
	pid_t childpidarray[CHILDS];
	const int n = 25;
	int status, i, j, k, m = 0, child, pivot;
	//int pipe01[2], pipe10[2], pipe02[2], pipe20[2], pipe03[2], pipe30[2], pipe04[2], pipe40[2], pipe05[2], pipe50[2];
	int parentTochild[CHILDS][2], childtoParent[CHILDS][2], received[CHILDS];
	char messageread[SIZE];
	char* messagewrite = (char*)malloc(80);

	srand((unsigned int)time(NULL));
	memset(childpidarray, -1, sizeof(childpidarray));
	for (i = 0; i < CHILDS; ++i) {
		if (pipe(parentTochild[i]) || pipe(childtoParent[i])) {
			perror("Could not create some pipe");
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < CHILDS; ++i) {
		if(!i || childpidarray[0] > 0) {
			childpidarray[i] = fork();
			if (childpidarray[i] < 0) {
				perror("Unable to fork some child");
				exit(EXIT_FAILURE);
			}
			if (childpidarray[i] == 0) {
				for (j = 0; j < i; ++j) childpidarray[j] = -1;
			}
		}
	}

	for (i = 0; i < CHILDS; ++i) {
		if (childpidarray[i] == 0) {
			dup2(parentTochild[i][0], STDIN_FILENO);
			dup2(childtoParent[i][1], STDOUT_FILENO);
			close(parentTochild[i][1]);
			close(childtoParent[i][0]);
			execl("child", "child", 0);
		}
		else if(childpidarray[i] > 0) {
			close(parentTochild[i][0]);
			close(childtoParent[i][1]);
		}
	}

	for (i = 0; i < CHILDS; ++i) {
		sprintf(messagewrite, "data%d.txt", i + 1);
		write(parentTochild[i][1], messagewrite, SIZE);
		read(childtoParent[i][0], messageread, SIZE);
		printf("Child %d sends READY\n", i + 1);
	}
	printf("Parent READY\n\n");

	k = n/2;
	while (1) {
		m = 0;
		do {
			child = rand()%5;
			sprintf(messagewrite, "%d", REQUEST);
			write(parentTochild[child][1], messagewrite, SIZE);	
			read(childtoParent[child][0], messageread, SIZE);
			pivot = atoi(messageread);
			printf("Child %d sends %d to Parent\n", child + 1, pivot);
		}while(pivot == -1);

		for (i = 0; i < CHILDS; ++i) {
			sprintf(messagewrite, "%d", PIVOT);
			write(parentTochild[i][1], messagewrite, SIZE);
			sprintf(messagewrite, "%d", pivot);
			write(parentTochild[i][1], messagewrite, SIZE);
		}
		printf("Parent broadcasts pivot %d to all children\n", pivot);
		for (i = 0; i < CHILDS; ++i) {
			read(childtoParent[i][0], messageread, SIZE);
			received[i] = atoi(messageread);
			printf("Child %d receives pivot and replies %d\n", i + 1, received[i]);
			m += received[i];
		}
		printf("Parent: %d = %d + %d + %d + %d + %d\n", m, received[0], received[1], received[2], received[3], received[4]);
		if (m != k) {
			sprintf(messagewrite, "%d", m < k ? LARGE : SMALL);
			if (m < k) printf("LARGE\n");
			for (i = 0; i < CHILDS; ++i) {
				write(parentTochild[i][1], messagewrite, SIZE);
			}
			if (m < k) k = k - m;
		}
		else break;
	}
	printf("Median Found = %d\n", pivot);
	for (i = 0; i < CHILDS; ++i) {
		kill(childpidarray[i], SIGUSR1);
	}
	printf("Parent sends kill signal to all children\n");
	for (i = 0; i < CHILDS; ++i) {
		waitpid(childpidarray[i], &status, 0);
		printf("Child %d terminates\n", i + 1);
	}
	exit(EXIT_SUCCESS);

}
