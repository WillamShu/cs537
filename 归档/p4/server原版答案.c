#include "cs537.h"
#include "request.h"
#include <pthread.h>

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

int *buffer; 
int use_ptr = 0;
int bufElementCount = 0;  
int bufSize;       
int fill_ptr = 0;

pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

/**
 * Get arguments from the command line.
 */
void getargs(int *port, int *threads, int *buffers, int argc, char *argv[])
{
    if (argc != 4) {
	fprintf(stderr, "Usage: %s <port> <threads> <buffers>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *buffers = atoi(argv[3]);
    // Check the arguments.
    if (*port <= 2000) {
    	fprintf(stderr, "Port number should be greater than 2000 to avoid active port.\n");
    	exit(1);
    }
    if (*threads <= 0) {
    	fprintf(stderr, "The number of worker threads must be a positive integer.\n");
    	exit(1);
    }
    if (*buffers <= 0) {
    	fprintf(stderr, "The number of request connections that can be accepted at one time must be a positive number.\n");
    	exit(1);
    }
}

/**
 * Put a value into the shared buffer.
 */
void put (int value) {
    buffer[fill_ptr] = value;
    fill_ptr = (fill_ptr + 1) % bufSize;
    bufElementCount++;
}

/**
 * Get a value out of the shared buffer.
 */
int get() {
    int tmp = buffer[use_ptr];
    use_ptr = (use_ptr + 1) % bufSize;
    bufElementCount--;
    return tmp;
}

/**
 * The main thread is the producer.
 */
void producer(int arg) {
    pthread_mutex_lock(&mutex);
    // Wait for the empty condition if the shared buffer is full.
    while (bufElementCount == bufSize) {
	pthread_cond_wait(&empty, &mutex);
    }
    // Store arg on the shared buffer.
    put(arg);
    pthread_cond_signal(&fill);
    pthread_mutex_unlock(&mutex);
}

/**
 * Multiple consumer threads will be created to handle the requests.
 */
void * consumer(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (bufElementCount == 0) {
            pthread_cond_wait(&fill, &mutex);
        }
        int tmp = get();
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
        requestHandle(tmp);
        Close(tmp);
    }
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads, buffers;
    struct sockaddr_in clientaddr;

    getargs(&port, &threads, &buffers, argc, argv);
    buffer = malloc(sizeof(int) * buffers);
    if (buffer == NULL) {
        fprintf(stderr, "malloc() failed.\n");
        exit(1);
    }
    bufSize = buffers;
    for (int i = 0; i < threads; i++) {
    	pthread_t thread;
    	pthread_create(&thread, NULL, consumer, NULL);
    }

    listenfd = Open_listenfd(port);
    while (1) {
    	clientlen = sizeof(clientaddr);
    	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    	producer(connfd);
    }

}






