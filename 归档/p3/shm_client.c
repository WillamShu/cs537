#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <string.h>
#define SHM_NAME "shaowen_wshu"

// Mutex variables
pthread_mutex_t* mutex;

typedef struct {
    int using;
    int pid;
    char birth[25];
    char clientString[10];
    int elapsed_sec;
    double elapsed_msec;
} stats_t;

stats_t *p;
char* start;
void exit_handler(int sig) {
    // critical section begins.
    pthread_mutex_lock(mutex);
    // Client leaving; needs to reset its segment 
    p->using = 0;
    pthread_mutex_unlock(mutex);
    // critical section ends.

    if (munmap(start, getpagesize()) == -1) {
        fprintf(stderr, "munmap() failed.\n");
        exit(1);
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    int fd_shm;
    struct timeval time1, time2;
    // handle signal
    struct sigaction sa; // all exit_handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = exit_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        fprintf(stderr, "SIGINT error.\n");
        exit(1);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        fprintf(stderr, "SIGTERM error.\n");
        exit(1);
    }
    fd_shm = shm_open(SHM_NAME, O_RDWR, 0660);
    if (fd_shm == -1) {
        fprintf(stderr, "shm_open() failed.\n");
        exit(1);
    }
    start = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if (start == MAP_FAILED) {
        fprintf(stderr, "mmap() failed.\n");
        exit(1);
    }
    mutex = (pthread_mutex_t*) start;
    stats_t *stats = (stats_t*) ((void*) start + sizeof(pthread_mutex_t));

    // critical section begins
    pthread_mutex_lock(mutex);
    // find valid page and invalidate segment
    for (int i = 0; i < 63; i++) {
        if (stats[i].using == 0) {
            stats[i].using = 1;
            stats[i].pid = getpid();
            time_t t;
            if (time(&t) == ((time_t) -1)) {
                fprintf(stderr, "Failure to obtain the current time.\n");
                exit(1);
            }
            char* cur_time;
            cur_time = ctime(&t);
            gettimeofday(&time1, NULL);
            if (cur_time == NULL) {
                fprintf(stderr, "Failure to convert the current time.\n");
                exit(1);
            }
            strncpy(stats[i].birth, cur_time, strlen(cur_time) - 1);
            strcpy(stats[i].clientString, argv[1]);
            p = stats + i;
            pthread_mutex_unlock(mutex);
            
            while (1) {
                // Print active clients
                gettimeofday(&time2, NULL);
                p->elapsed_sec = time2.tv_sec - time1.tv_sec;
                p->elapsed_msec = (time2.tv_usec - time1.tv_usec) / 1000.0f;
                sleep(1);
                fprintf(stdout, "Active clients :");
                for (int i = 0; i < 63; i++) {
                    if (stats[i].using == 1) 
                        fprintf(stdout, " %d ", stats[i].pid);
                }
                fprintf(stdout, "\n");
            }
        }
    }
    pthread_mutex_unlock(mutex);
    exit(1);
    // critical section ends

    return 0;
}
