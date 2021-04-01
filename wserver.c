#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "request.h"
#include "io_helper.h"
#include "cda.h"
#include "integer.h"


char default_root[] = ".";

typedef struct {
	CDA* buffer;
	pthread_mutex_t* bufferMutex;
	pthread_cond_t* emptyBuffer;
	pthread_cond_t* fullBuffer;
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
	/*
		Use circular dynamic array for storing file descriptors
		Useful for FIFO because insertions and removals are amortized constant
		For SFF removals are still amortized constant, but insertions are linear b/c of shifting
		I don't expect the buffer size to ever become big enough for this to be a problem
	*/
	CDA* buffer = newCDA();
	setCDAdisplay(buffer, displayINTEGER);
	setCDAfree(buffer, freeINTEGER);
	//initializing pthread stuff
	pthread_mutex_t bufferMutex;
	if(pthread_mutex_init(&bufferMutex, NULL) != 0){
		printf("mutex failed to be created\n");
		exit(1);
	}
	pthread_t threads[numThreads];
	pthread_cond_t emptyBuffer = PTHREAD_COND_INITIALIZER;
	pthread_cond_t fullBuffer = PTHREAD_COND_INITIALIZER;
	//initializing arg
	thread_arg* arg = (thread_arg*) malloc(sizeof(thread_arg));
	arg->buffer = buffer;
	arg->bufferMutex = &bufferMutex;
	arg->emptyBuffer = &emptyBuffer;
	arg->fullBuffer = &fullBuffer;
	//starting threads
	for(int i = 0; i < numThreads; i++){
		pthread_create(&threads[i], NULL, thread, arg);
	}

	while (1) {
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		printf("hi\n");
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		INTEGER* fd = newINTEGER(conn_fd);
		//inserting into array
		printf("we get this far\n");
		pthread_mutex_lock(&bufferMutex);
		while(sizeCDA(buffer) == bufferSize)
			pthread_cond_wait(&fullBuffer, &bufferMutex);
		if(schedulingAlgorithm == 0) //FIFO
			insertCDAback(buffer, fd);
		else //SFF
			insertCDA(buffer, binarySearch(buffer, conn_fd), fd);
		pthread_cond_signal(&emptyBuffer);
		printf("insetion done\n");
		pthread_mutex_unlock(&bufferMutex);
    }
	// displayCDA(buffer, stdout);
    return 0;
}

void* thread(void* a){
	thread_arg* arg = (thread_arg*) a;
	
	pthread_mutex_lock(arg->bufferMutex);
	while(sizeCDA(arg->buffer) == 0)
		pthread_cond_wait(arg->emptyBuffer, arg->bufferMutex);
	INTEGER* tempInt= (INTEGER*) removeCDAfront(arg->buffer);
	pthread_cond_signal(arg->fullBuffer);
	pthread_mutex_unlock(arg->bufferMutex);

	int conn_fd = getINTEGER(tempInt);
	request_handle(conn_fd);
	close_or_die(conn_fd);

	freeINTEGER(tempInt);
	return NULL;
}

int binarySearch(CDA* items, int key){
	int length = sizeCDA(items);
	if(length == 0)
		return 0;
	int guess = length/2;
	int oldGuess = -1;
	while(oldGuess != guess){
		// printf("old: %d, guess: %d, key %d ", oldGuess, guess, key);
		if(guess == length)
			break;
		if(getINTEGER((INTEGER*) getCDA(items, guess)) == key){
			// printf("case 1\n");
			oldGuess = guess;
			//break tie by age of request so that requests are less likely to starve
			if(++guess == length){
				break;
			}
		}
		else if(getINTEGER((INTEGER*) getCDA(items, guess)) > key){
			// printf("case 2\n");
			if(guess == 0)
				break;
			if(oldGuess == -1){
				guess = length/4;
				oldGuess = length/2;
			}
			else if(guess - oldGuess == 1)
				break;
			else if(oldGuess-guess == 1)
				oldGuess = guess--;
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
		else{
			// printf("case 3\n");
			if(oldGuess == -1){
				guess = (length + guess)/2 + 1;
				oldGuess = length/2;
			}
			else if(oldGuess - guess == 1){
				oldGuess = guess++;
				break;
			}
			else if(guess - oldGuess == 1){
				oldGuess = guess++;
			}
				
			else if(guess > oldGuess){
				int temp = guess;
				guess = guess + (guess-oldGuess)/2;
				oldGuess = temp;
			}
			else{
				int temp = guess;
				guess += (oldGuess-guess)/2;
				oldGuess = temp;
			}
		}
	}
	// printf("result: %d at %d\n", key, guess);
	return guess;
}