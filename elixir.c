#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

pid_t pid;
int killed = 0;
int stopped = 0;
char *exec_file;
char **new_argv;

void fork_and_exec()
{
	int status;
        pid = fork();

        if (pid == -1) {
                fprintf(stderr, "Cant fork\n");
                exit(1);
        } else if (pid == 0) {
		//The child
                status = execv(exec_file, new_argv);
		if (status == -1)
			exit(1);
	}
}

void signal_handler(int sig)
{
	fprintf(stderr, "**********************\n");
	fprintf(stderr, "* Elixir signaled!!! *\n");
	fprintf(stderr, "**********************\n");
	
	killed = 1;
	kill(pid, SIGINT);
	stopped = 1;
}

void child_handler(int sig)
{
	int status;
	if (waitpid(pid, &status, 0) < 0) {
		perror("waitpid()");
		exit(EXIT_FAILURE);
	}

	if (WIFEXITED(status)) {
		if (status == 0 && !killed) {
			fprintf(stderr, "***********************\n");
			fprintf(stderr, "* Restarting child!!! *\n");
			fprintf(stderr, "***********************\n");
			fork_and_exec();
		} else {
			stopped = 1;
		}
	} else {
		stopped = 1;
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [executable] [args...]\n");
		return 2;
	}
	exec_file = argv[1];

	new_argv = malloc(sizeof(char *) * argc);
	int i;
	for (i = 0; i < argc - 1; i++)
		new_argv[i] = argv[i + 1];
	new_argv[argc - 1] = NULL;

	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = signal_handler;
	sigaction(SIGTERM, &act, 0);
	sigaction(SIGINT, &act, 0);

	memset(&act, 0, sizeof(act));
	act.sa_handler = child_handler;
	sigaction(SIGCHLD, &act, 0);

	fork_and_exec();
	while (!stopped)
		sleep(1);
}
