all: shm_client shm_server

%: %.c
	gcc $< -o $@ -Wall -Werror -lrt -lpthread
