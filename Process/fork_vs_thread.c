/*
*
* This program uses Pthreads to creates a thread to executes the functionality of exec_linux_cmd.c 
* unlike a child process in fork.c 
* The objectives of your program are to:
* 1. Create a simple program that creates a thread.
* 2. Have this thread show all files and folders using the functionality built in exec_linux_cmd.c
* 3. Have the main thread wait for the created thread to finish execution.
*
* Threads Vs Child process:
*
* Child process is a process which is created by another process (it's parent process) but thread(s) form 
* a process, they can execute any part of a process code and they share memory space unlike processes which 
* run on seoarate memory spaces. Thread is the basic unit to which a slice of process time is allocated by the 
* Operating system.
*
*/



#include<stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<sys/types.h>
#include<unistd.h>

void *functionality()
{
	system("mkdir directory");
	system("ls /home/niket/TI2726-C/lab_assignments/");
	return NULL;
}

int main()
{
	pthread_t thread;
	int thread_return_id;

	thread_return_id = pthread_create( &thread, NULL, functionality, NULL);

     if(thread_return_id)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n", thread_return_id);
         exit(EXIT_FAILURE);
     }

     pthread_join( thread, NULL);
     
     return 0;
}