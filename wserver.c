#include <stdio.h>
#include <pthread.h>
#include "request.h"
#include "io_helper.h"
#include "cda.h"
#include "integer.h"


char default_root[] = ".";

typedef struct {
	CDA* buffer;
	
} thread_arg;

void* thread(void*);
int binarySearch(CDA* items, int key);

//
// ./wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg]
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
	int numThreads = 1;
	int bufferSize = 1;
	int schedulingAlgorithm = 0;
    
    while ((c = getopt(argc, argv, "d:p:t:b:s:")) != -1)
	switch (c) {
		case 'd':
			root_dir = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 't':
			numThreads = atoi(optarg);
			if(numThreads < 1){
				fprintf(stderr, "number of threads must be a positive integer\n");
				exit(1);
			}
			break;
		case 'b':
			bufferSize = atoi(optarg);
			if(bufferSize < 1){
				fprintf(stderr, "number of buffers must be positive integer\n");
				exit(1);
			}
			break;
		case 's':
			if(strcmp("SFF", optarg) == 0)
				schedulingAlgorithm = 1;
			else if(strcmp("FIFO", optarg) != 0){
				fprintf(stderr, "Scheduling algorithm must be either FIFO or SFF\n");
				exit(1);
			}
			break;
		default:
			fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg]\n");
			exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);
	CDA* buffer = newCDA();
	pthread_t threads[numThreads];
	thread_arg* arg = (thread_arg*) malloc(sizeof(thread_arg));
	// for(int i = 0; i < numThreads; i++){
	// 	pthread_create(&threads[i], NULL, thread, arg);
	// }

	insertCDAback(buffer,newINTEGER(1));
	insertCDAback(buffer,newINTEGER(2));
	insertCDAback(buffer,newINTEGER(3));
	insertCDAback(buffer,newINTEGER(4));
	insertCDAback(buffer,newINTEGER(5));
	insertCDAback(buffer,newINTEGER(5));

	printf("3 %d, 1 %d, 5 %d, 4 %d\n", binarySearch(buffer, 3), binarySearch(buffer, 1), binarySearch(buffer, 5), binarySearch(buffer, 4));
    // while (1) {
	// 	struct sockaddr_in client_addr;
	// 	int client_len = sizeof(client_addr);
	// 	int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
	// 	INTEGER* fd = newINTEGER(conn_fd);
	// 	if(schedulingAlgorithm == 0) //FIFO
	// 		insertCDAback(buffer, fd);
	// 	else{ //SFF

	// 	}
	// 	// request_handle(conn_fd);
	// 	// close_or_die(conn_fd);
    // }
    return 0;
}

void* thread(void* a){
	thread_arg* arg = (thread_arg*) a;
	
}

int binarySearch(CDA* items, int key){
	int length = sizeCDA(items);
	int guess = length;
	int oldGuess = 0;
	while(oldGuess != guess){
		if(getINTEGER((INTEGER*) getCDA(items, guess)) == key){
			oldGuess = guess;
			//break tie by age of request so that requests are less likely to starve
			if(++guess == length){
				break;
			}
		}
		else if(getINTEGER((INTEGER*) getCDA(items, guess)) > key){
			if(guess - oldGuess == 1)
				break;
			if(oldGuess-guess == 1)
				oldGuess = guess--;
			else if(guess > oldGuess){
				int temp = guess;
				guess = oldGuess + (guess-oldGuess)/2;
				oldGuess = temp;
			}
			else{
				int temp = guess;
				guess += (oldGuess-guess)/2;
				oldGuess = temp;
			}
		}
		else{
			if(oldGuess - guess == 1){
				oldGuess = guess++;
				break;
			}
			if(guess - oldGuess == 1)
				break;
			else if(guess > oldGuess){
				int temp = guess;
				guess = oldGuess + (guess-oldGuess)/2;
				oldGuess = temp;
			}
			else{
				int temp = guess;
				guess -= (oldGuess-guess)/2;
				oldGuess = temp;
			}
		}
	}
	return guess;
}