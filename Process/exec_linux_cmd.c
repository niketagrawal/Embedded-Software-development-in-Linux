/* This program implements the following tasks.
*
* 1. Create a directory in the directory in which the program is executed
* 2. List all files and directories in the current directory
* 3. Accomplish the above by making use of:
*   a) The possibility to invoke shell functions from C programs
*   b) The functionality for interaction with files and folders from the C standard libraries that are available through #include
*
*/


#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<dirent.h> 
#include <sys/stat.h>


int main()
{
	// Approach 1: Invoking shell functions

	system("mkdir directory");
	system("ls /home/niket/TI2726-C/lab_assignments/");

	//Approach 2: Using C standard libraries

	struct stat st = {0};
	if (stat("/home/niket/TI2726-C/lab_assignments/directory", &st) == -1)
	{
		mkdir("/home/niket/TI2726-C/lab_assignments/directory", 0700);
	}
	
	// To list the files in the current direcory
	
	DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d)
    {
    	while ((dir = readdir(d)) != NULL)
    	{
        	printf("%s\n", dir->d_name);
    	}
    	closedir(d);
  	}
		
    return 0;

}