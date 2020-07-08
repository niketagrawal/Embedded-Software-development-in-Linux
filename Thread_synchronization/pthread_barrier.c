/*
This program builds an application that let the LEDs on the Pi 'breath' in and out once. For this, it makes
use of barriers (PThreads supports these), and Pulse width modulation.
Objectives:
1. Create a thread that slowly fades the LEDs from unlit to lit.
2. Create a thread that slowly fades the LEDs from lit to unlit.
3. Start both threads at the same time.
4. Create a barrier and use this barrier to ensure the second thread does not start working before
the first thread has completed.
*/


#include<stdio.h>
#include<wiringPi.h>
#include <softPwm.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<math.h>
#include<time.h>

#define LED1 7
#define LED2 0
#define LED3 2
#define LED4 3

#define SLEEP_DURATION 50000

pthread_barrier_t mybarrier;

int flag = 0; 				//Set this flag to '1' when the callback_glow has executed once for the first time, thereafter
							//it will wait for the callbak_dim to get over

void init_wiringpi_leds()
{
	wiringPiSetup();
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);
}

void init_pwm_leds()
{
	softPwmCreate(LED1, 0, 100);
	softPwmCreate(LED2, 0, 100);
	softPwmCreate(LED3, 0, 100);
	softPwmCreate(LED4, 0, 100);
}

void update_leds(int value)
{
	softPwmWrite(LED1, value);
	softPwmWrite(LED2, value);
	softPwmWrite(LED3, value);
	softPwmWrite(LED4, value);
}


void *callback_glow()
{
	printf("callback_glow starts\n");	
	while(1)
	{
		if(flag)
    	{
        	pthread_barrier_wait(&mybarrier);
    	}
		int value = 0;
		while(value <= 100)
		{
			printf("glow value is %d\n", value);
			update_leds(value);
			usleep(SLEEP_DURATION);
			value++;
		}
		flag = 1;
		printf("glow phase ends\n");
		pthread_barrier_wait(&mybarrier);
	}
	return NULL;
}


void *callback_dim()
{
	printf("callback_dim starts\n");
	while(1)
	{
		pthread_barrier_wait(&mybarrier);
		printf("wait over, leds will dim now to zero\n");
    	int value = 100;
    	while(value >= 0)
    	{
			printf("dim value is %d\n", value);
    		update_leds(value);
        	usleep(SLEEP_DURATION);
        	value--;
    	}
		printf("dim phase ends\n");
		pthread_barrier_wait(&mybarrier);
	}
    	return NULL;
}



int main(int argc, char **argv)
{
	init_wiringpi_leds();

	init_pwm_leds();

    pthread_t glow_thread, dim_thread;
	pthread_barrier_init(&mybarrier, NULL, 2);

	int a, b;

    a = pthread_create(&glow_thread, NULL, callback_glow, NULL);

    if(a)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", a);
        exit(EXIT_FAILURE);
    }
	
	b = pthread_create(&dim_thread, NULL, callback_dim, NULL);	

	if(b)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", b);
        exit(EXIT_FAILURE);
    }

	pthread_join(glow_thread, NULL);
	pthread_join(dim_thread, NULL);
	pthread_barrier_destroy(&mybarrier);

	return 0;	
}
