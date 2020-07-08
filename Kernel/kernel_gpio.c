/*
This program creates a tiny kernel module that controls the GPIO pins on the Raspberry Pi. 
Resources: http://www.tldp.org/LDP/lkmpg/2.6/html/index.html
In order to control the GPIO pins in kernel space, gpio.h header file is used, which can be included 
by adding #include <linux/gpio.h>. The GPIO pins are requested before using them and freeing them when
the kernel module is cleaned up.
Since gpio.h is used, the following line is added to the code to enable the loading of the kernel module
: MODULE_LICENSE("GPL");

> Objectives:
1. Create a kernel module.

2. On initialization, have the kernel module request the GPIO pins with the LEDs, print some-
thing to the kernel logs, and turn on all the LEDs.

3. On removal, have the kernel module turn off all the LEDs, free the used GPIO pins, and print
something to the kernel logs.
*/

#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_INFO */
//#include <linux/init.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Niket Agrawal");

int init_module(void)
{
    printk(KERN_INFO "4_1: Request GPIO pins\n");

    //Allocating GPIO before use
    gpio_request(4, "4");
    gpio_request(17, "7");
    gpio_request(27, "27");
    gpio_request(22, "22");

    //gpio_request_array() //use to allocate for all 4 leds together
	printk(KERN_INFO "4_1: Setting GPIO pins to output\n");

    gpio_direction_output(4, 0);
    gpio_direction_output(17, 0);
    gpio_direction_output(27, 0);
    gpio_direction_output(22, 0);

    printk(KERN_INFO "4_1: Turning ON LEDs\n");

    gpio_set_value(4, 1);
    gpio_set_value(17, 1);
    gpio_set_value(27, 1);
    gpio_set_value(22, 1);
    
    return 0;
}

void cleanup_module(void)
{
    printk(KERN_INFO "4_1: Turning OFF LEDs\n");

    gpio_set_value(4, 0);
    gpio_set_value(17, 0);
    gpio_set_value(27, 0);
    gpio_set_value(22, 0);

    printk(KERN_INFO "4_1: freeing the GPIO pins\n");

    gpio_free(4);
    gpio_free(17);
    gpio_free(27);
    gpio_free(22);

    printk(KERN_INFO "4_1: cleanup done, exiting...\n");
}
