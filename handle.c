/*
 * Authors: Alexander Lo and Kamron Ledet
 * Date: September 11, 2016
 * This file prints "Still here" every second and utilizes
 * signals to initiate alternative behaviors.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "util.h"

/*
 * Signal handler prints "Nice try" and continues looping upon
 * receiving a SIGINT signal.
 *
 * sig represents the signal associated with the signal handler.
 */

void sigint_handler(int sig)
{
	//Alex driving now
	ssize_t bytes;
	const int STDOUT = 1;
	bytes = write(STDOUT, "Nice try.\n", 10);
	if(bytes != 10)
		exit(-999);
}

/*
 * Signal handler prints "exiting" and exits with value 1 upon
 * receiving a SIGUSR1 signal.
 *
 * sig represents the signal associated with the signal handler.
 */

void sigusr1_handler(int sig)
{
	//Alex driving now
	ssize_t bytes;
	const int STDOUT = 1;
	bytes = write(STDOUT, "exiting\n", 10);
	if(bytes != 10)
		exit(-999);
	exit(1);
}
 
/*
 * First, print out the process ID of this process.
 *
 * Then, set up the signal handler so that ^C causes
 * the program to print "Nice try.\n" and continue looping.
 *
 * Finally, loop forever, printing "Still here\n" once every
 * second.
 */
 
int main(int argc, char **argv)
{
	//Kamron driving now
	struct timespec time, time2;
	time.tv_sec = 1;
	time.tv_nsec = 0;
	int sleepdone = 0;
	
	/*
     * Catch SIGINT and SIGUSR1 signal and execute appropriate
     * signal handlers.
     */

	printf("%d\n", getpid());
	Signal(SIGINT, sigint_handler);
	Signal(SIGUSR1, sigusr1_handler);
	
	/*
     * Alex driving now
     * 
     * Loop forever and print "Still here" each second. Should a 
     * signal interrupt the process, run only the remaining time
     * after the interruption to fill 1 second.
     */

	while(1){
		if(sleepdone < 0)
			time = time2;
		else{
			time.tv_sec = 1;
			time.tv_nsec = 0;
			printf("Still here\n");
		}
		sleepdone = nanosleep(&time, &time2);
	}
	return 0;
}


