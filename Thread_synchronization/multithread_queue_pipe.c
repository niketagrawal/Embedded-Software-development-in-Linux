/*
This program re-uses the queue built in multithread_queue to provide data to the
LEDs. A small system is built that can easily PWM-control all four leds, where
each led has a separate queue and thread, and there is another fifth thread to handle user input. 
The output of one process is continuously provided as input to another using a pipe.
The file lightshow.txt stores numbers in the format LED(0-3) Brightness(0-100) Duration(ms). 
This text file is used to test the program.
Objectives:
1. Create four queues, one for each LED.
2. Create four threads, one for each LED, that fetch one by one the elements from their respective
queues and set the LED to the indicated brightness for the specified duration. When the
thread's queue is empty, turn off the corresponding LED.
3. Reuse the same functions for each LED. Avoid multiple functions with
the same functionality only for different LEDs.
4. Create one additional thread, which accepts command line input of the form LED(0-3)
Brightness(0-100) Duration(ms); these data are then added to the correct LED's
queue.
5. Ensure the program works correctly by piping output from the lightshow.txt into the program.

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

//#define SLEEP_DURATION 50000

int led_number;

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

void update_leds(int value, int led_number)
{
	switch(led_number)
	{
		case 0:
			softPwmWrite(LED1, value);
			break;
		case 1:
			softPwmWrite(LED2, value);
			break;
		case 2:
			softPwmWrite(LED3, value);
			break;
		case 3:
			softPwmWrite(LED4, value);
			break;
	}
	
}

typedef struct Node
{
	int brightness, duration; 
	struct Node* next;
}Node;

typedef struct Queue
{
	Node* front;			//First node in the queue
	Node* back;				//Last node in the queue
	pthread_mutex_t mutex;
	int led_number;
}Queue;

void initQueue(Queue* queue, int number)
{
	queue->front = NULL;
	queue->back = NULL;
	queue->led_number = number;
}

Queue *queue_1, *queue_2, *queue_3, *queue_4;

/*typedef struct Map
{
	int led_number;
	Queue *queue;
}Map;
*/

/*  Adds a new node to the back of the queue
*/
void addToQueue(Queue* queue, int brightness, int duration)
{	
//	printf("Inside addtoqueue\n");
	pthread_mutex_lock(&(queue->mutex));
	Node *new_node = (Node *)malloc(sizeof(Node));
	new_node->brightness = brightness;
	new_node->duration = duration;
	new_node->next = NULL;
	if(queue->front == NULL)
	{
		printf("xxx\n");
		queue->front = new_node;
		queue->back = new_node;
		pthread_mutex_unlock(&(queue->mutex));
		return;
	}
	Node *temp = queue->back;
	temp->next = new_node;
	queue->back = 	new_node;
	pthread_mutex_unlock(&(queue->mutex));
//	printf("Retuning from addtoqueue\n");	
}

/* Removes a node from the front of the queue
*  The memory of the removed node has to be freed
*  Returns the values of the removed node via pointers
*  If the queue is empty return -1 for both the brightness and the duration
*/
void removeFromQueue(Queue* queue, int* pBrightness, int* pDuration)
{
//	printf("Inside removefromqueue\n");
	pthread_mutex_lock(&(queue->mutex));
	if((queue->front==NULL) && (queue->back==NULL))
	{
		printf("2\n");
		*pBrightness = -1;
		*pDuration = -1;
		pthread_mutex_unlock(&(queue->mutex));
		return;
	}
	Node* temp = queue->front;
	*pBrightness = temp->brightness;
	*pDuration = temp->duration;
	queue->front = queue->front->next;
	free(temp);
	pthread_mutex_unlock(&(queue->mutex));
//	printf("Retuning from removefromqueue\n");
}

//Returns the length of the queue
int queueSize(Queue* queue)
{
	int size = 0;
	if(queue->front==NULL)
	{
		size = 0;
		return size;
	}
	Node *temp = queue->front;

	while(temp != (queue->back))
	{
		temp = temp->next;
		size++;
	}
	return (size+1);
}

void *callback_led(void *queue)
{
	Queue *queue_temp = (Queue *)queue;

	int a= 0, b = 0;
	int *pBrightness = &a;
	int *pDuration = &b;
			
	while(1)
	{
		while(queueSize(queue_temp) != 0)
		{
			removeFromQueue(queue_temp, pBrightness, pDuration);
			update_leds(*pBrightness, queue_temp->led_number);
			printf("%d %d\n", *pBrightness, queue_temp->led_number);
			usleep(*pDuration * 1000);
		}
		update_leds(0, queue_temp->led_number);		
	}		
	
	return NULL;
}


void *callback_userinput()
{
	int led_number, brightness;
	double duration;
	while(scanf("%d %d %lf", &led_number, &brightness, &duration) != EOF)
	{
	//	scanf("%d %d %lf", &led_number, &brightness, &duration);
		switch(led_number)
		{
			case 0:
				addToQueue(queue_1, brightness, duration);
				break;
			case 1:
				addToQueue(queue_2, brightness, duration);
				break;
			case 2:
				addToQueue(queue_3, brightness, duration);
				break;
			case 3:
				addToQueue(queue_4, brightness, duration);
				break;
		}
		
	}

	return NULL;
}


int main(int argc, char **argv)
{	
	init_wiringpi_leds();
	init_pwm_leds();

	queue_1 = (Queue *)malloc(sizeof(Queue));
	queue_2 = (Queue *)malloc(sizeof(Queue));
	queue_3 = (Queue *)malloc(sizeof(Queue));
	queue_4 = (Queue *)malloc(sizeof(Queue));

	initQueue(queue_1, 0);
	initQueue(queue_2, 1);
	initQueue(queue_3, 2);
	initQueue(queue_4, 3);
	
	pthread_t thread_1, thread_2, thread_3, thread_4, thread_5;

	if (pthread_mutex_init(&(queue_1->mutex), NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
 
	if (pthread_mutex_init(&(queue_2->mutex), NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    if (pthread_mutex_init(&(queue_3->mutex), NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    if (pthread_mutex_init(&(queue_4->mutex), NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }


	int a, b, c, d, e;

    a = pthread_create(&thread_1, NULL, callback_led, (void *)queue_1);

    if(a)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", a);
        exit(EXIT_FAILURE);
    }
	
	b = pthread_create(&thread_2, NULL, callback_led, (void *)queue_2);	

	if(b)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", b);
        exit(EXIT_FAILURE);
    }

	//size = queueSize(queue);
	//printf("Size of queue after add operation is %d\n", size);

	c = pthread_create(&thread_3, NULL, callback_led, (void *)queue_3);

    if(c)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", c);
        exit(EXIT_FAILURE);
    }
    
    d = pthread_create(&thread_4, NULL, callback_led, (void *)queue_4);  

    if(d)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", d);
        exit(EXIT_FAILURE);
    }

    e = pthread_create(&thread_5, NULL, callback_userinput, NULL);  

    if(e)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", e);
        exit(EXIT_FAILURE);
    }

    pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);
	pthread_join(thread_3, NULL);
    pthread_join(thread_4, NULL);
    pthread_join(thread_5, NULL);

	//size = queueSize(queue);
	//printf("Size of queue after remove operation is %d\n", size);

    pthread_mutex_destroy(&(queue_1->mutex));
    pthread_mutex_destroy(&(queue_2->mutex));
    pthread_mutex_destroy(&(queue_3->mutex));
    pthread_mutex_destroy(&(queue_4->mutex));

	return 0;	
}
