/* 
 * msh - A mini shell program with job control
 * 
 * <Put your name and login ID here>
 *
 * Authors: Alexander Lo (atlo1126)
 * and Kamron Ledet (kledet)
 * Date: September 14-15, 2016
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
#include "jobs.h"


/* Global variables */
int verbose = 0;            /* if true, print additional output */

extern char **environ;      /* defined in libc */
static char prompt[] = "msh> ";    /* command line prompt (DO NOT CHANGE) */
static struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

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

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

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
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
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
	char buf[MAXLINE];
	int bg;
	sigset_t mask;
	pid_t pid;
	
	/*
	 * Prepare the command line for execution and check to make
	 * sure that something has been inputted
	 */

	strcpy(buf, cmdline);
	bg = parseline(buf, argv);
	if(argv[0] == NULL)
		return;

	/*
	 * If the command line does not contain a built-in command,
	 * child process will be created to execute the inputted
	 * command. Use sigprocmask to block and unblock SIGCHLD
	 * signals and setpgid to put the child in a new process group.
	 */
	
	if(!builtin_cmd(argv)){
		sigemptyset(&mask);
		sigaddset(&mask, SIGCHLD);
		sigprocmask(SIG_BLOCK, &mask, NULL);
		if((pid = fork()) == 0){
			setpgid(0,0);
			sigprocmask(SIG_UNBLOCK, &mask, NULL);
			if(execve(argv[0], argv, environ) < 0){
				printf("%s: Command not found.\n", argv[0]);
				exit(0);
			}
		}

		/*
		 * Kamron driving now
		 * 
		 * Add job to foreground or background as specified or
		 * kills process if adding the job fails. Print pid and
		 * jid if job is to run in the background.
		 */

		if(!bg){	
			if(addjob(jobs, pid, FG, cmdline)){
				sigprocmask(SIG_UNBLOCK, &mask, NULL);
				waitfg(pid);
			}
			else
				kill(-pid, SIGINT);
		}
		else{
			if(addjob(jobs, pid, BG, cmdline)){
				sigprocmask(SIG_UNBLOCK, &mask, NULL);
				printf("[%d] (%d) %s", pid2jid(jobs, pid), pid, cmdline);
			}
			else
				kill(-pid, SIGINT);
		}
		
	}
    return;
}


/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 * Return 1 if a builtin command was executed; return 0
 * if the argument passed in is *not* a builtin command.
 *
 * partial builtin_cmd code from Computer Systems, A Programmer's Perspective 3rd ed.
 * Page 755
 *
 * argv represents the array that contains the command and its arguments.
 */
int builtin_cmd(char **argv) 
{	
	/*
	 * Alex driving now
	 * 
	 * exit with value 0 if built-in command equals "quit"
	 * print jobs if built-in command equals "jobs"
	 * run in background/foreground if command equals "bg" or "fg"
	 * ignore singleton & if command equals "&"
	 */

	if(!strcmp(argv[0], "quit"))
		exit(0);
	if(!strcmp(argv[0], "jobs")){
		listjobs(jobs);
		return 1;
	}
	if(!strcmp(argv[0], "bg")){
		do_bgfg(argv);
		return 1;
	}
	if(!strcmp(argv[0], "fg")){
		do_bgfg(argv);
		return 1;
	}
	if(!strcmp(argv[0], "&"))
		return 1;
    return 0;     /* not a builtin command */
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 *
 * argv represents the array that contains the command and its arguments.
 */
void do_bgfg(char **argv) 
{
	//Kamron driving now
	struct job_t *job;
	
	//Check for PID or JID as argument

	if(argv[1] == NULL){
		printf("%s command requires PID or %%jobid argument\n", argv[0]);
		return;
	}
	
	/*
	 * Check for the existence of the job with the specified PID/JID.
	 * Determine PID/JID by the presence of a %
	 */

	if(isdigit(argv[1][0])){
		if(!(job = getjobpid(jobs, atoi(argv[1])))){
			printf("(%d): No such process\n", atoi(argv[1]));
			return;
		}
	}
	else if(argv[1][0] == '%'){
		if(!(job = getjobjid(jobs, atoi(&argv[1][1])))){
			printf("%s: No such job\n", argv[1]);
			return;
		}
	}
	else{
		printf("%s: argument must be a PID or %%jobid\n", argv[0]);
		return;
	}

	/*
	 * Send SIGCONT signal to specified PID and change state of job to
	 * BG or FG as specified. Print PID and JID if job is in the background.
	 */
	
	kill(-job->pid, SIGCONT);
		
	if(!strcmp(argv[0], "bg")){
		job->state = BG;
		printf("[%d] (%d) %s", job->jid, job->pid, job->cmdline);
	}
	else if(!strcmp(argv[0], "fg")){
		job->state = FG;
		waitfg(job->pid);
	}
    return;
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 *
 * pid represents the PID of the job that needs to be waited on.
 */
void waitfg(pid_t pid)
{
	//Alex driving now
	sigset_t mask, oldmask;
	
	/*
	 * Utilize sigprocmask in order to suspend the shell until a signal
	 * has been received.
	 */

	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	
	sigprocmask(SIG_BLOCK, &mask, &oldmask);

	//Loop checks for an existing foreground job.

	while(fgpid(jobs) == pid){
		sigsuspend(&oldmask);
	}
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	
    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 *
 * sig represents the signal associated with the signal handler.
 */
void sigchld_handler(int sig) 
{
	//Kamron driving now
	pid_t pid;
	int status;
	ssize_t bytes;
	char str[100];
	struct job_t *job;
	
	//Loop waits for all of the children to terminate.

	while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0){
		/*
		 * WIFSIGNALED checks if the child was terminated by a signal.
		 * Print JID, PID, and signal number.
		 */
		if(WIFSIGNALED(status)){
			sprintf(str, "Job [%d] (%d) terminated by signal %d\n", pid2jid(jobs, pid), pid, WTERMSIG(status));
			bytes = write(1, str, strlen(str));
			if(bytes != strlen(str))
				exit(-999);
		}
		/*
		 * WIFSTOPPED checks if the child was suspended by a signal.
		 * Print JID, PID, and signal number and change state to STOP.
		 */
		else if(WIFSTOPPED(status)){
			sprintf(str, "Job [%d] (%d) stopped by signal %d\n", pid2jid(jobs, pid), pid, WSTOPSIG(status));
			bytes = write(1, str, strlen(str));
			if(bytes != strlen(str))
				exit(-999);
			job = getjobpid(jobs, pid);
			job->state = ST;
			return;
		}
		//Delete job if not suspended.
		deletejob(jobs, pid);
	}

    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job. 
 *
 * sig represents the signal associated with the signal handler. 
 */
void sigint_handler(int sig) 
{
	//Alex driving here
	pid_t fgid = fgpid(jobs);
	if(fgid){
		kill(-fgid, SIGINT);
	}
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 *
 * sig represents the signal associated with the signal handler.
 */
void sigtstp_handler(int sig) 
{
	//Alex driving here
	pid_t fgid = fgpid(jobs);
	if(fgid){
		kill(-fgid, SIGTSTP);
	}
    return;
}

/*********************
 * End signal handlers
 *********************/



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



