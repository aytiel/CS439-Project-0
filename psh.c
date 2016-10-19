/* 
 * psh - A prototype tiny shell program with job control
 * 
 * <Put your name and login ID here>
 *
 * Authors: Alexander Lo (atlo1126)
 * and Kamron Ledet (kledet)
 * Date: September 11, 2016
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
#include "util.h"



/* Global variables */
int verbose = 0;            /* if true, print additional output */

extern char **environ;      /* defined in libc */
static char prompt[] = "psh> ";    /* command line prompt (DO NOT CHANGE) */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);

/* Here are helper routines that we've provided for you */
void usage(void);
void sigquit_handler(int sig);



/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }


    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.
 *
 * eval code from Computer Systems, A Programmer's Perspective 3rd ed.
 * Page 755 
 *
 * cmdline represents the inputted command and arguments as a string.
 */
void eval(char *cmdline) 
{
	//Alex driving now
	char *argv[MAXARGS];
	pid_t pid;
	
	/*
	 * Prepare the command line for execution and check to make
	 * sure that something has been inputted
	 */
	 
	parseline(cmdline, argv);
	if(argv[0] == NULL)
		return;
	
	/*
	 * If the command line does not contain a built-in command,
	 * child process will be created to execute the inputted
	 * command
	 */
	 
	if(!builtin_cmd(argv)){
		if((pid = fork()) == 0){
			if(execve(argv[0], argv, environ) < 0){
				printf("%s: Command not found. \n", argv[0]);
				exit(0);
			}
		}
		
		//Parent process waits for child process to finish
		
		int status;
		if(waitpid(pid, &status, 0) < 0)
			unix_error("waitfg: waitpid error");
	}
	return;
}


/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately. 
 * Return 1 if a builtin command was executed; return 0
 * if the argument passed in is *not* a builtin command.
 *
 * argv represents the array that contains the command and its arguments.
 */
int builtin_cmd(char **argv) 
{
	//exit with value 0 if built-in command equals "quit"
	
	if(!strcmp(argv[0], "quit"))
		exit(0);
    return 0;     /* not a builtin command */
}





/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    ssize_t bytes; 
    const int STDOUT = 1; 
    bytes = write(STDOUT, "Terminating after receipt of SIGQUIT signal\n", 45); 
    if(bytes != 45) 
       exit(-999);
    exit(1);
}



