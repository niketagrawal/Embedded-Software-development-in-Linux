/*
This project consists of designing a C program to serve as a shell interface 
that accepts user commands and then executes each command in a separate process.

Reference http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/exec.html

A shell interface gives the user a prompt, after which the next command is entered. 
The example below illustrates the prompt osh>
and the user’s next command, which displays the file prog.c on the terminal using the Linux cat
command:

osh> cat prog.c

One technique for implementing a shell interface is to have the parent process first read what the
user enters on the command line (in this case, cat prog.c), and then create a separate child process
that performs the command. Unless otherwise specified, the parent process waits for the child
to exit before continuing. However, Linux shells typically also allow the child process to run in
the background, or concurrently. To accomplish this, we add an ampersand (&) at the end of the
command. Thus, if we rewrite the above command as

osh> cat prog.c &

the parent and child processes will run concurrently. The separate child process is created using the
fork() system call, and the user’s command is executed using one of the system calls in the exec()
family of system calls. A C program that provides the general operations of a command-line shell
3is supplied below. The main() function presents the prompt osh-> and outlines the steps to be taken
after input from the user has been read. The main() function continually loops as long as the boolean
should run equals 1; when the user enters exit at the prompt, your program will set should run to 0
and terminate.

This exercise is organised into two parts:

Part 1: Creating a Child Process

The first task is to modify the main() function as mentioned above so that a child process is forked
and executes the command specified by the user. This will require parsing what the user has entered
into separate tokens and storing the tokens in an array of character strings. For example, if the user
enters the command ps -ael at the osh> prompt, the values stored in the args array are:

args[0] = "ps"
args[1] = "-ael"
args[2] = NULL

This args array will be passed to the execvp() function, which has the following prototype:
execvp(char *command, char *params[]);
Here, command represents the command to be performed and params stores the parameters to this
command. For this project, the execvp() function should be invoked as execvp(args[0], args). Be
sure to check whether the user included an & to determine whether or not the parent process is to
wait for the child to exit.

Part 2: Creating a History Feature
The next task is to modify the shell interface program so that it provides a history feature that allows
the user to access the most recently entered commands. The user will be able to access up to 10
commands by using the feature. The commands will be consecutively numbered starting at 1, and
the numbering will continue past 10. For example, if the user has entered 35 commands, the 10 most
recent commands will be numbered 26 to 35. The user will be able to list the command history by
entering the command history at the osh> prompt. As an example, assume that the history
consists of the commands (in order of execution):
ps; ls -l; top; cal; who; date

The command history will output:
1
2
3
4
5
6
ps
ls -l
top
cal
who
date

This the same as the history command on the normal terminal of the Pi, except that we want you
to just print the last 10 commands.
Your program should support two techniques for retrieving commands from the command history:
1. When the user enters !!, the most recent command in the history is executed.
2. When the user enters a single ! followed by an integer N, the Nth command in the history is
executed.
3. (In the normal terminal on the Pi there is also the option !-X which would execute the X to last
command, so !-1 would repeat the last command. You don’t have to implement this feature,
but it might be useful to know.)
Continuing our example from above, if the user enters !!, the ps command will be performed; if the
user enters !3, the command top will be executed. Any command executed in this fashion should
be echoed on the user’s screen. The command should also be placed in the history buffer as the
next command. The program should also manage basic error handling. If there are no commands in
the history, entering !! should result in a message No commands in history. If there is no command
corresponding to the number entered with the single !, the program should output No such command
in history.
*/


#include <stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<string.h>
#include <unistd.h>
#define MAX_LINE 80 				/* The maximum length command */


void  parse(char *string, char **args)      /*Function to parse and store the user input command*/
{
     while (*string != '\0')                /*End of line check*/
     {       					
        while (*string == ' ' || *string == '\t' || *string == '\n')    /* replacing whilte spaces with '\0' character*/
        {
        	*string++ = '\0';  
        }

        *args++ = string;                               /*save the sub-string in an array index*/

        while (*string != '\0' && *string != ' ' && *string != '\t' && *string != '\n')  
        {
        	string++;                  /*continue until no white space or end of line is encountered*/
        }                   					
     }
     *args = '\0';                 				
}

void  execute(char **args)
{
     pid_t pid;
     int status, i=0;

     pid = fork();

     if (pid < 0) 			 
     {     																		
          printf("fork Failed" );
          exit(1);
     }
     else if (pid == 0)  					  /*child process*/
     {          
          if (execvp(*args, args) < 0)        
          {
              printf("execvp Failed" );
        	  exit(1);
          }
     }
     else 
     {    
     	while(args[i] != NULL)
     	{
     		i++;                 /*Reach the end of the array*/
     	}
     	if(*args[i-1] != '&')    /*check for the '&' character at the end of the command stored in the array*/
     	{
     		printf("\nNo & found in the user command, parent process waiting for child to terminate\n");
     		while (wait(&status) != pid);       /*Waits infinitely for the child process to complete*/
     	}                       
     	else
     	{
     		printf("\n& found so child process is running in background\n");
     		exit(0);
     	}                   
     }
}

int main(void) 
{
	char *args[MAX_LINE/2 +1]; 		/* command line arguments  */
	char input_command[MAX_LINE];	/* Command string input by the user */
	int should_run = 1; 			/* flag to determine when to exit program */
	
	while (should_run) 
	{
		printf("osh>");
		fflush(stdout);
		gets(input_command);
		parse(input_command, args);

		if (strcmp(args[0], "exit") == 0)
		{
			exit(0);
		}

		execute(args);
	}
	return 0;
}