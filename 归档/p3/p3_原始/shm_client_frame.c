#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#define PAGESIZE 4096
// ADD NECESSARY HEADERS

// Mutex variables
pthread_mutex_t* mutex;

void exit_handler(int sig) {
	// ADD

	// critical section begins
	pthread_mutex_lock(mutex);

	// Client leaving; needs to reset its segment

	pthread_mutex_unlock(mutex);
	// critical section ends

	exit(0);
}

int main(int argc, char *argv[]) {
	// ADD
	int fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0660);
	char *shm_ptr = (char* )mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
	char *string = strdup("Hello World");
	strcpy(shm_ptr, string);

	// critical section begins
	pthread_mutex_lock(mutex);

	// client updates available segment

	pthread_mutex_unlock(mutex);
	// critical section ends

	while (1) {

		// ADD

		sleep(1);

		// Print active clients
	}

	return 0;
}






