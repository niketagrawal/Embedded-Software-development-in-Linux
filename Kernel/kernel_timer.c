/*

Software timers are a concept in Linux that allows to schedule the execution of a function in
the code at a later point in time. Timers can be used, for example, to regularly poll the status of a
device that does not generate interrupts. Normally linux/timers more than suffices. This uses
jiffies 5 as a unit of time. On the Raspberry Pi, jiffies span somewhere slightly above
10ms. For the programs in this repository, this is a bit too large!
Thankfully, linux/hrtimer.h exists, which provides high-resolution timers accurate down to
nanoseconds. HrTimers works with a unit called KTime, which is located in linux/ktime.
HrTimers are represented by a structure hrtimer. This structure has a number of properties, such
as a callback function and an expiration time.

This prpogram creates a kernel module that uses hrtimers to turn on and off the
LEDs on the Pi in a certain pattern. 

Objectives:
1. Create a kernel module.
2. Use a hrtimer to turn the LEDs on for one second and then off for two seconds. Keep repeating
this as long as the module is loaded.
*/

#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_INFO */
//#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>


MODULE_LICENSE("GPL");

static struct hrtimer hr_timer;
static int flag = 0;		//for maintaining 1 sec and 2 sec period for LED toggle
ktime_t currtime, time, interval;

enum hrtimer_restart hrTimerCallback(struct hrtimer *timer_for_restart)
{
    currtime = ktime_get();
	if((flag % 2) == 0)
	{	
    	gpio_set_value(4, 1);
    	gpio_set_value(17, 1);
    	gpio_set_value(27, 1);
    	gpio_set_value(22, 1);  
        // Set an interval of 1 second, which is equal to 1*10^9 nanosecond
        interval = ktime_set(0, 1e9);
	}
	else
	{	
    	gpio_set_value(4, 0);
    	gpio_set_value(17, 0);
    	gpio_set_value(27, 0);
    	gpio_set_value(22, 0);  
		interval = ktime_set(0, 2e9);	
	}

	// Move restart timer to a time in the future
    hrtimer_forward(timer_for_restart, currtime, interval);
	flag++;
    return HRTIMER_RESTART;
}

int init_module(void)
{
	time = ktime_set( 0, 1e9);

    printk(KERN_INFO "4_3 Request GPIO pins\n");

	gpio_request(4, "4");
    gpio_request(17, "7");
    gpio_request(27, "27");
    gpio_request(22, "22");

    printk(KERN_INFO "4_3 Setting GPIO pins to output\n");

    gpio_direction_output(4, 0);
    gpio_direction_output(17, 0);
    gpio_direction_output(27, 0);
    gpio_direction_output(22, 0);

	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);    
	hr_timer.function = &hrTimerCallback;

	// Start a hrtimer
	hrtimer_start( &hr_timer, time, HRTIMER_MODE_REL);
	
    return 0;
}

void cleanup_module(void)
{
    printk(KERN_INFO "4_3 cleanup is being done\n");

	// Cancel a hrtimer
	hrtimer_cancel(&hr_timer);

    printk(KERN_INFO "4_3 freeing the GPIO pins\n");

    gpio_free(4);
    gpio_free(17);
    gpio_free(27);
    gpio_free(22);

	printk(KERN_INFO "4_3 cleanup done, exiting\n");
}


