/*
 * Authors: Alexander Lo and Kamron Ledet
 * Date: September 9, 2016
 * This file computes the nth number of the Fibonacci sequence using fork() to emulate recursion with child processes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

const int MAX = 13;

static void doFib(int n, int doPrint);


/*
 * unix_error - unix-style error routine.
 */
inline static void 
unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}


int main(int argc, char **argv)
{
  	int arg;
  	int print;

  	if(argc != 2){
    	fprintf(stderr, "Usage: fib <num>\n");
    	exit(-1);
  	}

  	arg = atoi(argv[1]);
  	if(arg < 0 || arg > MAX){
		fprintf(stderr, "number must be between 0 and %d\n", MAX);
		exit(-1);
	}

	doFib(arg, 1);

	return 0;
}

/* 
 * Recursively compute the specified number. If print is
 * true, print it. Otherwise, provide it to my parent process.
 *
 * NOTE: The solution must be recursive and it must fork
 * a new child for each call. Each process should call
 * doFib() exactly once.
 *
 * n represents the desired number of the Fibonacci sequence.
 * doPrint identifies the parent process.
 */
static void 
doFib(int n, int doPrint)
{
    // Alex driving now
	int ans;
	int status;
	pid_t pid1, pid2;
	int v1 = 0, v2 = 0;

	/*
     * Kamron driving now
	 * Base case for n=0: print 0 if the original input 
	 * was 0, exit 0 otherwise
	 */
 
	if(n == 0){
		if(doPrint){
			printf("0\n");	
			return;
		}
		else
			exit(0);
	}

	/*
	 * Base case for n=1: print 1 if the original input 
	 * was 1, exit 1 otherwise
	 */
 
	if(n == 1){
		if(doPrint){
			printf("1\n");
			return;
		}
		else
			exit(1);
	}

	/*
	 * Alex driving now
	 * Fork twice for the n-1 and n-2 factors of the current member 
	 * of the Fibonacci sequence
	 */
 
	pid1 = fork();
	if(pid1 == 0){
		doFib(n-1, 0);
	}

	/*
	 * Recursive calls are done inside here in the child process only, 
	 * will be terminated by an eventual exit call
	 */

	pid2 = fork();
	if(pid2 == 0){
		doFib(n-2, 0);
	}

	/*
	 * In the parent process, waits for the two child processes to terminate 
	 * and grabs their exit statuses
	 */

	waitpid(pid1, &status, 0);
	v1 = WEXITSTATUS(status);
	
	waitpid(pid2, &status, 0);
	v2 = WEXITSTATUS(status);

	/*
	 * Kamron driving now
	 * The two results of n-1 and n-2 are added together
	 */

	ans = v1 + v2;

	//Print the final answer if doPrint is true, or instead exit to parent 
	if(doPrint){
		printf("%d\n", ans);
	}
	else
		exit(ans);
}


