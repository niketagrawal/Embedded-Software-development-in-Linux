/*
This program creates a character device driver. This character device driver registers itself 
as /dev/klog, accepts string input, and prints this input to the kernel log in a readable format. 
Use printk() to print to the kernel log.

Objectives:
1. Create a character device driver that registers as /dev/klog.
2. Let strings written to this character device driver be printed to the kernel log.
3. Ensure the character device driver de-registers and unloads correctly.
4. Make sure that the /dev/klog file can only be opened by one application at the same time.
When the file already is opened and another program tries to open it, fopen() should return
an error code in that program. Make sure that this mechanism is thread safe.
The device driver can de tested from the terminal using the following command:
sudo sh -c 'printf "Your message" > /dev/klog'
*/

#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_INFO */
//#include <linux/init.h>
//#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <linux/device.h>

#define BUF_LEN 1024
#define DEVICE_NAME "device_klog" 		//this will appear as /dev/device_klog after kernel module is loaded
										//Device name 'klog' is not available. This was detected during debugging, 
										//the device_write function was not hit so Device name was chosen as different

MODULE_LICENSE("GPL");

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

struct mutex dev_mut;
static int major_number;
static struct class *class_ptr;
static char user_data[BUF_LEN] = {0};
static int busy = 0;
static int i;

static struct file_operations fops = 
{
    .open = device_open,
    .release = device_release,
    .write = device_write
};


int init_module(void)
{
    mutex_init(&dev_mut); 
	
	major_number = register_chrdev(0, DEVICE_NAME, &fops);

	if (major_number < 0) 
	{
	  printk(KERN_ALERT "4_2: Registering char device failed with %d\n", major_number);
	  return major_number;
	}

	printk(KERN_INFO "4_2: I was assigned major numberr %d. To talk to\n", major_number);

	class_ptr = class_create (THIS_MODULE, "4_2class");

	device_create (class_ptr, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    
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
		printk(KERN_ALERT "4_2: Error in unregister_chrdev: %d\n", ret);
	}
*/
	mutex_destroy(&dev_mut);

	printk(KERN_INFO "4_2: clean up done, exiting\n");
}

static int device_open(struct inode *inode, struct file *file)
{
	//mutex_lock(&dev_mut);
    printk(KERN_INFO "4_2: inside device open\n");
    if (busy)
    {
        
        return -EBUSY;
    }
    printk(KERN_INFO "4_2: device_open fine\n");
    busy++;

 //   try_module_get(THIS_MODULE);

    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "4_2: inside device release\n");


    if(busy)
    {
        busy--;
    }
     
    //  Decrement the usage count, or else once you opened the file, you'll
     // never get get rid of the module. 
     
//    module_put(THIS_MODULE);

//	mutex_unlock(&dev_mut);
    return 0;
}

static ssize_t device_write(struct file *filp, const char *buffer, size_t length, loff_t *off)
{
	printk(KERN_INFO "4_2: Entered Device write_1\n");
    mutex_lock(&dev_mut);
    printk(KERN_INFO "4_2: Entered Device write, taken lock\n");
    for (i = 0; i < length; i++)
    {
        get_user(user_data[i], buffer++);
    }

    printk(KERN_INFO "4_2: Entered data is %s\n", user_data);

    mutex_unlock(&dev_mut);
   // return (ssize_t)i;
    return length;
}

