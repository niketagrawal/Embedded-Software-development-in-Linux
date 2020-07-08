/*
*
* This program extends the functionality in multithreading_counter_compute.c to investigate the differences
* between busy waiting and sleeping and looks into thread priorities.
*
* Objectives:
*   1. Replace usleep() with busyWait(), observe the difference in execution time of
*      the work thread and ensure the LEDs can still be controlled
*   3. While still using busyWait the execution of the thread that reads the filecan be speeded up
*      by giving it a higher priority than the other two threads. Give the other two threads the same
*      lower priority and use Round Robin scheduling. Each thread can be given a priority and
*      policy using the snippet above. Again, look at the execution time.
*   4. In a short comment explain why the LEDs do not function correctly as long as the thread that
       reads the file (with a higher priority) is not yet finished.

*/

/*

In busy waiting/spinning, a process repeatedly check for a condition if it is true
In sleep(), process actually goes to sleep, is woken up again after the desired time.
Time incurred in waking up ang going to sleep unlike busy waiting 

*/




#include<stdio.h>
#include<wiringPi.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<math.h>
#include<time.h>

#define LED1 7
#define LED2 0
#define LED3 2
#define LED4 3


void busyWait(int s)
{
	clock_t now = clock();
	while (clock() < now + s) {};
}

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
//    usleep(200000);
	busyWait(200000);
    displayled(temp);
}

void *read_file()
{
    FILE *fp;
    fp = fopen("/home/pi/TI2726-C/Multithreading/data.txt", "r");

    if(fp == NULL)
    {
        printf("Error in opening file");
        exit(1);
    }
//  char value[10];
    double value;
//  double values[10];
//    unsigned int i;       
 /*
    for(i = 0; fgetc(fp) != EOF; ++i)
    {
        fscanf(fp, "%lf", &values[i]);
        printf("%lf\n", values[i]);
    }
 */
    double sum = 0;
    //time_t start = time(NULL);
    clock_t start = clock();

    while(fgetc(fp) != EOF)
    {
        fscanf(fp, "%lf", &value);
        value = atan(tan(value));
        sum = sum + value;
    }
//  time_t end = time(NULL);
//  time_t total = (double)(end - start);

    clock_t end = clock();
    clock_t total = (double)(end - start);

    printf("sum is %lf\n", sum);
    printf("No. of clock ticks elapsed = %ld", total);
    printf("Time elapsed = %ld", total/1000000);

    fclose(fp);
    return NULL;

}

void *count_thread_function(void *goal)
{
	while((*(int *)goal) != -1)
    {
        job_count(*(int *)goal);
    }
    return NULL;
}


int main(int argc, char **argv)
{
    wiringPiSetup();

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);

    int goal = 0;

    pthread_t listen_thread, count_thread;

	pthread_t fileread_thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr); //Initialize thread attributes
    //Makes sure the thread does not inherit the attributes of its parent:
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    //Sets the scheduling policy to Round Robin:
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    struct sched_param param;
    param.sched_priority = 10; //Sets the priority of the thread
    //Includes the priority in the attribute:
    pthread_attr_setschedparam(&attr, &param);
    //Creates the thread with the defined policy and priority:
    pthread_create(&fileread_thread, &attr, read_file, NULL);
	

	pthread_attr_t attr_2;
    pthread_attr_init(&attr_2); //Initialize thread attributes
    //Makes sure the thread does not inherit the attributes of its parent:
    pthread_attr_setinheritsched(&attr_2, PTHREAD_EXPLICIT_SCHED);
    //Sets the scheduling policy to Round Robin:
    pthread_attr_setschedpolicy(&attr_2, SCHED_RR);
	struct sched_param param_listenthread;
	param_listenthread.sched_priority = 2;
	pthread_attr_setschedparam(&attr_2, &param_listenthread);
	pthread_create(&listen_thread, &attr_2, job_listen, (void *)&goal);


	pthread_attr_t attr_3;
    pthread_attr_init(&attr_3); //Initialize thread attributes
    //Makes sure the thread does not inherit the attributes of its parent:
    pthread_attr_setinheritsched(&attr_3, PTHREAD_EXPLICIT_SCHED);
    //Sets the scheduling policy to Round Robin:
    pthread_attr_setschedpolicy(&attr_3, SCHED_RR);
	struct sched_param param_countthread;
	param_countthread.sched_priority = 2;
	pthread_attr_setschedparam(&attr_3, &param_countthread);

	pthread_create(&count_thread, &attr_3, count_thread_function, (void *)&goal);

	pthread_join( listen_thread, NULL);
	pthread_join( fileread_thread, NULL);

	return 0;
}

