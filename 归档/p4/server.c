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

pthread_cond_t non = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int bufSize = -1;    
int bufElementCount = 0;     
int cons = 0;
int prod = 0;
int *buffer; 

void producer(int val) {
    pthread_mutex_lock(&mutex);
    while (bufElementCount == bufSize) {
       pthread_cond_wait(&non, &mutex);
    }
    buffer[prod] = val;
    prod = (prod + 1) % bufSize;
    bufElementCount++;
    pthread_cond_signal(&full);
    pthread_mutex_unlock(&mutex);
}

void * consumer(void *val) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (bufElementCount == 0) {
            pthread_cond_wait(&full, &mutex);
        }
        int data = buffer[cons];
        cons = (cons + 1) % bufSize;
        bufElementCount--;
        pthread_cond_signal(&non);
        pthread_mutex_unlock(&mutex);
        requestHandle(data);
        Close(data);
    }
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    int thr, buf;
    struct sockaddr_in clientaddr;

    if (argc != 4) {
        fprintf(stderr, "Error: Invalid num of args.\n");
        exit(1);
    }
    port = atoi(argv[1]);
    thr = atoi(argv[2]);
    buf = atoi(argv[3]);
    if ( ! (port > 2000 && thr > 0 && buf > 0) ) {
        fprintf(stderr, "Error: Invalid args content.\n");
        exit(1);
    }

    buffer = malloc(sizeof(int) * buf);
    if (buffer == NULL) {
        fprintf(stderr, "Failed allocation in malloc().\n");
        exit(1);
    }

    bufSize = buf;
    for (int i = 0; i < thr; i++) {
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