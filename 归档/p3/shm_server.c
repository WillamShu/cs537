#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#define SHM_NAME "shaowen_wshu"

// Mutex variables
pthread_mutex_t* mutex;
pthread_mutexattr_t mutexAttribute;

typedef struct {
    int using;
    int pid;
    char birth[25];
    char clientString[10];
    int elapsed_sec;
    double elapsed_msec;
} stats_t;

char * start;
void exit_handler(int sig) {
    // When server terminates, ensure that it correctly removes the shared memory page
    if (munmap(start, getpagesize()) == -1) {
        fprintf(stderr, "munmap() failed.\n");
        exit(1);
    }
    if (shm_unlink(SHM_NAME) == -1) {
        fprintf(stderr, "shm_unlink() failed.\n");
        exit(1);
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    int fd_shm;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = exit_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        fprintf(stderr, "SIGINT error.\n");
        exit(1);
    }

    // create and initialize the shared memory page
    fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0660);
    if (fd_shm == -1) {
        fprintf(stderr, "shm_open() failed.\n");
        exit(1);
    }

    // cause this file to be truncated to a precise size in bytes
    int pageSize = 4096;
    if (ftruncate(fd_shm, pageSize) == -1) {
        fprintf(stderr, "fruncate() failed.\n");
        exit(1);
    }

    // mapping  shared memory segment to the address space
    start = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if (start == MAP_FAILED) {
        fprintf(stderr, "mmap() failed.\n");
        exit(1);
    }

    // useing mutex to point to the shared memory page.
    mutex = (pthread_mutex_t*) start;
    if (pthread_mutexattr_init(&mutexAttribute) != 0) {
        fprintf(stderr, "pthread_mutexattr_init() failed.\n");
        exit(1);
    }
    // Initializing mutex
    pthread_mutexattr_setpshared(&mutexAttribute, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &mutexAttribute);
    stats_t *stats = (stats_t*) ((void*) start + sizeof(pthread_mutex_t));
    int counter = 1;

    // enter an infinite loop where it continually sleeps for one second,
    // reads the contents of shared memory, and then displays the statistics
    // that have been written to shared memory to STDOUT.
    while (1) {
        for (int i = 0; i < 63; i++) {
            if (stats[i].using == 1) {
                fprintf(stdout, "%d, ", counter);
                fprintf(stdout, "pid : %d, ", stats[i].pid);
                fprintf(stdout, "birth : %s, ", stats[i].birth);
                fprintf(stdout, "elapsed : %d s %.4f ms, ", stats[i].elapsed_sec, stats[i].elapsed_msec);
                fprintf(stdout, "%s\n", stats[i].clientString);
            }
        }
        counter++;
        sleep(1);
    }

    return 0;
}
