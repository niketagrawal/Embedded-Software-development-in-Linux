/*
* This program extends the funtionality in multithreading_counter.c with a third thread running at the
* same time as the other two threads. This thread reads lightshow.txt filled with doubles, and performs 
* a computation on these numbers.
* Objectives:
*  1. Create a thread that reads a file (of different sizes) with a double on each line.
*  2. On this same thread, for every number n in the file, calculate n = atan(tan(n)).
*  3. On this same thread, sum all these numbers, and print the result, as well as the time it took to
*     complete the entire calculation
*  4. Ensure the other two threads still function correctly while processing the file.
*
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

void *read_file()
{
	FILE *fp;
	fp = fopen("/home/pi/TI2726-C/Multithreading/data.txt", "r");

	if(fp == NULL)
	{
		printf("Error in opening file");
		exit(1);
	}
//	char value[10];
	double value;
//	double values[10];
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
//	time_t end = time(NULL);
//	time_t total = (double)(end - start);

	clock_t end = clock();
    clock_t total = (double)(end - start);

	printf("sum is %lf\n", sum);
	printf("No. of clock ticks elapsed = %ld", total);
	printf("Time elapsed = %ld", total/1000000);

    fclose(fp);
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

    pthread_t listen_thread, fileread_thread;

    int a, b;

    a = pthread_create(&listen_thread, NULL, job_listen, (void *)&goal);

    if(a)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", a);
        exit(EXIT_FAILURE);
    }

	b = pthread_create(&fileread_thread, NULL, read_file, (void *)&goal);	

	if(b)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", b);
        exit(EXIT_FAILURE);
    }



    while(goal!= -1)
    {
//      printf("inside main, goal is %d\n", goal);
        job_count(goal);
    }

    return 0;
}


















