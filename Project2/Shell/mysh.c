#include <stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdbool.h>
#include<ctype.h>

#define BUFFER_SIZE 10240
#define SIZE 2000

int split_execute(char *input);

void check_for_exit(char *response);

void print_error();

void split_by_space(char* input, char *argv[]);

int back_counter = 0;
int back_queue[SIZE];
int counter = 1;
char input[BUFFER_SIZE];
char string[128];

// output error message
void print_error() {
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));

}

// split by space
void split_by_space(char* input, char *argv[]) {
	int count = 0;
	char* token;
	char* delim = " \n";
	token = strtok(input, delim);
	while (token != NULL) {
		argv[count] = token;
		count++;
		token = strtok(NULL, delim);
	}
	argv[count] = NULL;
}

// check if execute split function
int split_execute(char *input) {
	char *argv[512];
	char *redirect_outfile = NULL;
	char *redirect_infile = NULL;

	split_by_space(input, argv);

	int loopi = 0;
	int endofarg = 0;
	int pipei = -1;
	int background = -1;
	while (argv[loopi] != NULL) {
		if (strcmp(argv[loopi], ">") == 0) {
			if (endofarg == 0) endofarg = loopi;
			if (argv[loopi + 1] == NULL || (argv[loopi + 2] != NULL && strcmp(argv[loopi + 2], "<") && strcmp(argv[loopi + 2], "&"))) {
				print_error();
				return 1;
			}
			redirect_outfile = argv[loopi + 1];
		}

		if (strcmp(argv[loopi], "<") == 0) {
			if (endofarg == 0) endofarg = loopi;
			if (argv[loopi + 1] == NULL || (argv[loopi + 2] != NULL && strcmp(argv[loopi + 2], ">") && strcmp(argv[loopi + 2], "&"))) {
				print_error();
				return 1;
			}
			redirect_infile = argv[loopi + 1];
		}
		if (strcmp(argv[loopi], "|") == 0) {
			pipei = loopi;
		}
		if (strcmp(argv[loopi], "&") == 0) {
			background = loopi;
		}
		loopi++;
	}
	if (pipei > -1) {
		if (pipei == 0 || argv[pipei + 1] == NULL) {
			print_error();
			return 1;
		}
	}
	if (redirect_outfile != NULL || redirect_infile != NULL) {
		argv[endofarg] = NULL;
	}
	/*create new process*/
	if (strcmp(argv[0],  "exit") == 0) {
		int i;
		for (i = 0; i < back_counter; ++i) {
			kill(back_queue[i], 10);
		}
		exit(0);
	} else if (strcmp(argv[0],  "cd") == 0) {
		if (argv[1] == NULL) {
			chdir(getenv("HOME"));
		} else {
			if (chdir(argv[1]) == -1 ) {
				print_error();
			}
		}
		return 0;
	} else if (strcmp(argv[0], "pwd") == 0) {
		if (argv[1] != NULL) {
			print_error();
		} else {
			printf("%s\n", getcwd(string, 128));
			fflush(stdout);
		}
		return 0;
	}
	int rc = fork();

	if (rc > 0) {
		if (background == -1) {
			waitpid(rc, NULL, 0);
		} else {
			back_queue[back_counter] = rc;
			back_counter++;
		}
	} else if (rc == 0) {
		if (pipei > -1) {
			int pipefd[2];
			if (pipe(pipefd) < 0) {
				print_error();
				exit(1);
			}
			int rcc = fork();
			if (rcc > 0) {
				close(pipefd[1]);
				dup2(pipefd[0], STDIN_FILENO);
				execvp(argv[pipei + 1], argv + pipei + 1);
				print_error();
				exit(1);
			} else if (rcc == 0) {
				close(pipefd[0]);
				dup2(pipefd[1], STDOUT_FILENO);
				argv[pipei] = NULL;
				execvp(argv[0], argv);
				print_error();
				exit(1);
			} else {
				print_error();
				exit(1);
			}
		}
		if (redirect_outfile != NULL) {
			close(STDOUT_FILENO);
			int ret = open(redirect_outfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
			if (ret == -1) {
				print_error();
				exit(1);
			}
			dup2(ret, STDOUT_FILENO);
		}
		if (redirect_infile != NULL) {
			close(STDIN_FILENO);
			int ret = open(redirect_infile, O_RDONLY, S_IRWXU);
			if (ret == -1) {
				print_error();
				exit(1);
			}
			dup2(ret, STDIN_FILENO);
		}
		if (background > -1) {
			argv[background] = NULL;
		}
		execvp(argv[0], argv);
		print_error();
		exit(0);

	} else {
		print_error();
	}
	return 0;
}

// trim the white space
char *trim_space(char *str)
{
	while (isspace(*str)) str++;
	return str;
}

int main(int argc, char *argv[]) {
	if (argc > 1) {
		print_error();
		exit(1);
	}
	int counter = 1;
	int r;

	while (1) {
		snprintf(string, 128, "mysh (%d)> ", counter);

		write(STDOUT_FILENO, string, strlen(string));

		if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
			print_error();
		}
		char* afterspace = trim_space(input);
		if (*afterspace == 0) continue;
		counter++;
		if (strlen(input) > 128) {
			print_error();
			continue;
		}

		split_execute(afterspace);

		int i;
		for (i = 0; i < back_counter; ++i) {
			waitpid(back_queue[i], NULL, WNOHANG);
		}
	}
}
