#include <stdio.h>
#include <pthread.h>
#include "request.h"
#include "io_helper.h"

char default_root[] = ".";

typedef struct {
	int listen_fd;
	struct sockaddr_in client_addr;
} thread_arg;

void* thread(void*);

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
    // while (1) {
		struct sockaddr_in client_addr;
		thread_arg* arg = (thread_arg*) malloc(sizeof(thread_arg));
		arg->client_addr = client_addr;
		arg->listen_fd = listen_fd;
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, thread, arg);
		pthread_join(thread_id, NULL);
		// int client_len = sizeof(client_addr);
		// int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		// request_handle(conn_fd);
		// close_or_die(conn_fd);
    // }
    return 0;
}

void* thread(void* a){
	thread_arg* arg = (thread_arg*) a;
	int client_len = sizeof(arg->client_addr);
	int conn_fd = accept_or_die(arg->listen_fd, (sockaddr_t *) &(arg->client_addr), (socklen_t *) &client_len);
	request_handle(conn_fd);
	close_or_die(conn_fd);
	return NULL;
}