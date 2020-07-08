/*
This program creates a character device driver that registers under /dev/pwm. When
an integer between 0 and 100 is written to the file, it uses PWM with a duty cycle between 0% and
100% to dim all LEDs as a result. Timers are used to implement the PWM.

Objectives:
1. Create a character device driver that registers under /dev/pwm.
2. Let integers between 0-100 written to this character device change the duty cycle between 0%
and 100%.
3. Every change in duty cycle should be printed to the kernel log.
4. Use PWM with the given duty cycle to set all LEDs to an intensity between 0% and 100%.
5. The period of one PWM cycle should be 10ms.
6. The timer callback function should not be called more than twice every PWM period.
7. Ensure the character device driver de-registers and unloads correctly.
*/

#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_INFO */
//#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <linux/device.h>

//#define BUF_LEN 1024
#define DEVICE_NAME "device_pwm"

MODULE_LICENSE("GPL");

static struct hrtimer hr_timer;
ktime_t currtime, time, interval;

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
//static ssize_t device_read(struct file *, char *, size_t, loff_t *);

struct mutex dev_mut;
static int major_number;
static struct class *class_ptr;
//static char user_data[BUF_LEN];
//static char *msg_Ptr;
static int busy = 0;
//static int i, 
static int flag = 0;
static unsigned long int duty_cycle;
static int return_value;
//static int counter = 0;

static struct file_operations fops = 
{
    .open = device_open,
    .release = device_release,
    .write = device_write,
//	.read = device_read
};


static int device_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "4_4: Entered device open\n");
    if (busy)
    {		
        return -EBUSY;
    }

    busy++;
 //   try_module_get(THIS_MODULE);
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    if(busy)
    {
        busy--;
    }
    /* 
     * Decrement the usage count, or else once you opened the file, you'll
     * never get get rid of the module. 
     */
 //   module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_write(struct file *filp, const char *buffer, size_t length, loff_t *off)
{
	printk(KERN_INFO "4_4: Entered device write\n");

    mutex_lock(&dev_mut);
/*
    for (i = counter; i < length && i < BUF_LEN; i++)
    {
        get_user(user_data[i], buffer + i);
    }i
	counter = length;
*/
	//memcpy(msg_Ptr, user_data, i);

	return_value = kstrtol(buffer, 10, &duty_cycle); 
/*
	if(duty_cycle)
	{
		hrtimer_start( &hr_timer, time, HRTIMER_MODE_REL);		
	}	
*/
    printk(KERN_INFO "4_4: Entered data is %ld\n", duty_cycle);

    mutex_unlock(&dev_mut);

	return length;
//    return (ssize_t)i;
}

/*
static ssize_t device_read(struct file *filp,	* see include/linux/fs.h   */
//{
	/*
	 * Number of bytes actually written to the buffer 
	 */
//	int bytes_read = 0;

	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
//	if (*msg_Ptr == 0)
//		return 0;

	/* 
	 * Actually put the data into the buffer 
	 */
//	while (length && *msg_Ptr) {
//
		/* 
		 * The buffer is in the user data segment, not the kernel 
		 * segment so "*" assignment won't work.  We have to use 
		 * put_user which copies data from the kernel data segment to
		 * the user data segment. 
		 */
//		put_user(*(msg_Ptr++), (char *)duty_cycle);

//		length--;
//		bytes_read++;
//	}

	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
/*	if(bytes_read)
	{
		hrtimer_start( &hr_timer, time, HRTIMER_MODE_REL);		
	}
*/
//	return bytes_read;
//}

enum hrtimer_restart hrTimerCallback(struct hrtimer *timer_for_restart)
{
	//printk(KERN_INFO "4_4: inside timer callback\n");

    currtime = ktime_get();
	if((flag % 2) == 0)
	{	
    	gpio_set_value(4, 1);
    	gpio_set_value(17, 1);
    	gpio_set_value(27, 1);
    	gpio_set_value(22, 1);  
        // Set an interval of 1 second, which is equal to 1*10^9 nanosecond
        //interval = ktime_set(0, (*duty_cycle)*1e3);
		interval = ktime_set(0, duty_cycle*100000);
	}
	else
	{	
    	gpio_set_value(4, 0);
    	gpio_set_value(17, 0);
    	gpio_set_value(27, 0);
    	gpio_set_value(22, 0);  
		//interval = ktime_set(0, (100-(*duty_cycle))*1e3);	
		interval = ktime_set(0, (100-duty_cycle)*100000);
	}

	// Move restart timer to a time in the future
    hrtimer_forward(timer_for_restart, currtime, interval);
	flag++;
    return HRTIMER_RESTART;
}

int init_module(void)
{
	time = ktime_set( 0, 0);
    mutex_init(&dev_mut); 
	
	major_number = register_chrdev(0, DEVICE_NAME, &fops);

	if (major_number < 0) 
	{
	  printk(KERN_ALERT "4_4: Registering char device failed with %d\n", major_number);
	  return major_number;
	}

	printk(KERN_INFO "4_4: I was assigned major numberr %d. To talk to\n", major_number);

	class_ptr = class_create (THIS_MODULE, "4_4class");

	device_create (class_ptr, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);

    
	gpio_request(4, "4");
    gpio_request(17, "7");
    gpio_request(27, "27");
    gpio_request(22, "22");

    printk(KERN_INFO "4_4: Setting GPIO pins to output\n");

    gpio_direction_output(4, 0);
    gpio_direction_output(17, 0);
    gpio_direction_output(27, 0);
    gpio_direction_output(22, 0);

	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);    
	hr_timer.function = &hrTimerCallback;
	
//	a = device_read(struct file *, char *, size_t, loff_t *);
	// Start a hrtimer
	hrtimer_start( &hr_timer, time, HRTIMER_MODE_REL);

    return 0;
}

void cleanup_module(void)
{
	// remove the device
	device_destroy(class_ptr, MKDEV(major_number, 0));
	// unregister the device class
	class_unregister(class_ptr);
	// remove the device class
	class_destroy(class_ptr);
	// unregister the major number
	unregister_chrdev(major_number, DEVICE_NAME);
	/*
	if (ret < 0)
	{
		printk(KERN_ALERT "4_3: Error in unregister_chrdev: %d\n", ret);
	}
*/
	mutex_destroy(&dev_mut);

	printk(KERN_INFO "4_3: clean up done, exiting\n");
}
