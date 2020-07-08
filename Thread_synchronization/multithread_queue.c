/*
Objectives for this program are:
1. Implement a queue Data structure to provide data to the LEDs. 
2. To ensure multiple threads can use the queue at the same time, make adding
and removing elements thread-safe through the use of mutual exclusion.
2. Use the mutex to make the functions thread safe: the functions can be called from different
threads without problems.
3. Spawn two threads, and have these threads both add 100,000 elements to the same queue
simultaneously.
4. Afterwards, print the size of the queue and make sure it is 200,000 elements.
55. Spawn two threads, and have these threads both remove 100,000 elements from the same
queue simultaneously.
6. Each time removeFromQueue is run, check if -1 is returned for the brightness and dura-
tion. This should not be the case, because the same amount of nodes is added and removed.
7. Afterwards, print the size of the queue and make sure it is 0 elements.

NOTE: It's important to use mutex in this program so that at a time only one thread adds the node to the queue.
	  If another thread tries to add node to the same queue at the same time, there would be problems because,
	  two threads would try to access and update the same resoource which is the front and back pointer of the queue
	  in this case
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
}Queue;

Queue *queue;

void initQueue(Queue* queue)
{
	queue->front = NULL;
	queue->back = NULL;
}

/*  Adds a new node to the back of the queue
*/
void addToQueue(Queue* queue, int brightness, int duration)
{
	pthread_mutex_lock(&(queue->mutex));
	Node *new_node = (Node *)malloc(sizeof(Node));
	new_node->brightness = brightness;
	new_node->duration = duration;
	new_node->next = NULL;
	if(queue->front == NULL)
	{
		queue->front = new_node;
		queue->back = new_node;
		pthread_mutex_unlock(&(queue->mutex));
		return;
	}
	Node *temp = queue->back;
	temp->next = new_node;
	queue->back = 	new_node;
	pthread_mutex_unlock(&(queue->mutex));
}

/* Removes a node from the front of the queue
*  The memory of the removed node has to be freed
*  Returns the values of the removed node via pointers
*  If the queue is empty return -1 for both the brightness and the duration
*/
void removeFromQueue(Queue* queue, int* pBrightness, int* pDuration)
{
	pthread_mutex_lock(&(queue->mutex));
	if((queue->front==NULL) && (queue->back==NULL))
	{
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

void *callback_add(void *queue)
{
	Queue *queue_temp = (Queue *)queue; 	
	int randomnumber_brightness, randomnumber_duration, i;
	
	for(i = 0; i < 10000; i++)
	{
		randomnumber_brightness = rand()%100;
    	randomnumber_duration = (rand()%10000)+10000;
		addToQueue(queue_temp, randomnumber_brightness, randomnumber_duration);  	
	}

	return NULL;
}

void *callback_remove(void *queue)
{
	Queue *queue_temp = (Queue *)queue;
	printf("Inside callback_3\n");
	int a = 0, b = 0, i;
	int *pBrightness = &a;
	int *pDuration = &b;

	for(i = 0; i < 10000; i++)
	{
		removeFromQueue(queue_temp, pBrightness, pDuration); 
		if((*pBrightness == -1) && (*pDuration == -1))
		{
			printf("Trying to remove from an empty queue!\n");
		}	
	}
	return NULL;
}

int main(int argc, char **argv)
{
	init_wiringpi_leds();
	init_pwm_leds();
	
	queue = (Queue *)malloc(sizeof(Queue));
	initQueue(queue);
	
	pthread_t thread_1, thread_2, thread_3, thread_4;

	if (pthread_mutex_init(&(queue->mutex), NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
 
	int a, b, c, d, size;

    a = pthread_create(&thread_1, NULL, callback_add, (void *)queue);

    if(a)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", a);
        exit(EXIT_FAILURE);
    }
	
	b = pthread_create(&thread_2, NULL, callback_add, (void *)queue);	

	if(b)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", b);
        exit(EXIT_FAILURE);
    }

	pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);
	
	size = queueSize(queue);
	printf("Size of queue after add operation is %d\n", size);

	c = pthread_create(&thread_3, NULL, callback_remove, (void *)queue);

    if(c)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", a);
        exit(EXIT_FAILURE);
    }
    
    d = pthread_create(&thread_4, NULL, callback_remove, (void *)queue);  

    if(d)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", b);
        exit(EXIT_FAILURE);
    }

	pthread_join(thread_3, NULL);
    pthread_join(thread_4, NULL);

	size = queueSize(queue);
	printf("Size of queue after remove operation is %d\n", size);

    pthread_mutex_destroy(&(queue->mutex));

	return 0;	
}
