/*
 * Authors: Alexander Lo and Kamron Ledet
 * Date: September 11, 2016
 * This file sends the SIGUSR1 signal to the specified pid.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>


int main(int argc, char **argv)
{
	//Kamron driving now
	pid_t pid;
	int killerr;
	
	//exit if incorrect amount of arguments

	if(argc != 2){
		fprintf(stderr, "Usage: ./mykill <PID>\n");
		exit(-1);
  	}
	
	/*
	 * obtain pid from command line input and send SIGUSR1
	 * signal to pid
	 */

	pid = atoi(argv[1]);
	killerr = kill(pid, SIGUSR1);

	//print error if sending signal not successful
	
	if(killerr < 0){
		fprintf(stderr, "Invalid PID\n");
		exit(-1);
	}
	
	return 0;
}
