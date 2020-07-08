/*
*
* This program extends the program in fork.c with the functionality to create and run
* another process. The child process executes the same code as the main program.
* The objectives of this program are to:
* 1. Create a single program that creates a new process running the same code using the function fork.
* 2. Create an if/else statement to print which process is active (child or parent)
* 3. The child and parent processes should do different things:
*    a) Make the child process show all files and folders using the functionality built in exec_linux_cmd.c
*    b) Have the parent process wait for the child process to finish.
*
*/


#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<dirent.h> 
#include<sys/stat.h>
#include<sys/wait.h>


int main()
{
	pid_t pid;

	pid = fork();

	if (pid < 0)
    {
        fprintf(stderr, "fork Failed" );
        return 1;
    }

    else if(pid > 0)  						//Parent process
    {
    	wait(NULL);
    	printf("Child process has terminated\n");
    }

	else    								//Child process (PID == 0)
	{
		system("mkdir directory");
		system("ls /home/niket/TI2726-C/lab_assignments/");
		exit(0);
	}
	

    return 0;

}