#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define SIZE 80
#define ARRAYSIZE 5
#define READY 0
#define REQUEST 1
#define PIVOT 2
#define SMALL 3
#define LARGE 4
#define SHOW 10
#define DELETED -1

void quit(){
	exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
	char *messagewrite = (char*)malloc(80), *datafile = (char*)malloc(9);
	char messageread[SIZE];
	FILE* fin;
	int array[ARRAYSIZE], i, message, pivot, n = ARRAYSIZE;
	srand((unsigned int)time(NULL));
	signal(SIGUSR1, quit);

	read(STDIN_FILENO, datafile, SIZE);
	if (!(fin = fopen(datafile, "r"))) {
		perror("Could not open file");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < ARRAYSIZE; ++i) {
		fscanf(fin, "%d", array + i);
	}
	sprintf(messagewrite, "%d", READY);
	write(STDOUT_FILENO, messagewrite, SIZE);

	while(1) {
		read(STDIN_FILENO, messageread, SIZE);
		message = atoi(messageread);
		switch(message) {
			case REQUEST:
				if(n > 0) {
					do {
						i = rand()%5;
						message = array[i];
					}while(message == -1);
				}
				else message = -1;
				sprintf(messagewrite, "%d", message);
				write(STDOUT_FILENO, messagewrite, SIZE);
				break;
			case PIVOT:
				read(STDIN_FILENO, messageread, SIZE);
				pivot = atoi(messageread);
				message = 0;
				for(i = 0; i < ARRAYSIZE; ++i){
					if (array[i] != -1 && array[i] > pivot)
						message++;
				}
				sprintf(messagewrite, "%d", message);
				write(STDOUT_FILENO, messagewrite, SIZE);
				break;
			case SMALL:
				for(i = 0; i < ARRAYSIZE; ++i) {
					if(array[i] != -1 && array[i] < pivot) {
						array[i] = -1;
						n--;
					}
				}
				break;
			case LARGE:
				for(i = 0; i < ARRAYSIZE; ++i) {
					if(array[i] != -1 && array[i] > pivot) {
						array[i] = -1;
						n--;
					}
				}
				break;
		}
	}

	fclose(fin);
	exit(EXIT_SUCCESS);
}
