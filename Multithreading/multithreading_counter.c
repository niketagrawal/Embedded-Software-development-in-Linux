/*
*
* This program builds an application that accepts numbers between 0 and 15 as input
* while the application is running, and then has the Pi’s LEDs count towards this number over time.
* The number is only allowed to go up or down one step every 200ms. When another number is entered
* as input while the application is running, the LEDs must count towards that number instead.
* The program creates separate threads for reading input, and for counting in binary with the LEDs. 
* Objectives:
*  1. Create a thread that continually listens to command line input, and checks if it is in the valid
*     range [0, 15]. Display an error message if this is not the case. Use this number as the goal of
*     the counter.
*  2. Create a thread that incrementally counts towards the goal number (provided by the other
*    thread), allowing only a change of -1 or +1 every 200 milliseconds. On the same
*    thread display the current number in binary using the Pi’s LEDs.
*  3. When a new goal is entered before the current one is reached, count to the new goal instead.
*  4. The program should keep listening to new goal values and counting to them (even after the
*     first goal is reached) until the goal -1 is entered. When -1 is entered both threads should stop.
*/



#include<stdio.h>
#include<wiringPi.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

#define LED1 7
#define LED2 0
#define LED3 2
#define LED4 3


void displayled(int number)
{
    digitalWrite(LED1, ((number >> 3) & 1));   //LED1 is the MSB and LED4 is LSB
    digitalWrite(LED2, ((number >> 2) & 1));
    digitalWrite(LED3, ((number >> 1) & 1));
    digitalWrite(LED4, (number & 1));
}


void *job_listen(void *ptr)
{
    int *number, input;
	number = (int *)ptr;

	while(*number != -1)
	{
		scanf("%d", &input);

    	if((input > 15) || (input < -1))
    	{
        	printf("Invalid input\n");
    	}
		else
		{
    		*number = input;
			printf("goal is %d\n", *(int *)ptr);
		}
	}
    
	return NULL;
}

void job_count(int goal)
{
    static int temp = 0;

	if(temp < goal)
	{
		temp++;
	}
	else if(temp > goal)
	{
		temp--;
	}
	usleep(200000);
	displayled(temp);
}


int main(int argc, char **argv)
{
    wiringPiSetup();

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);

	int goal = 0;

    pthread_t listen_thread;

	int thread_check;

	thread_check = pthread_create(&listen_thread, NULL, job_listen, (void *)&goal);

    if(thread_check)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", thread_check);
        exit(EXIT_FAILURE);
    }	


	while(goal!= -1)
	{
 		job_count(goal);
	}
   
    return 0;
}


