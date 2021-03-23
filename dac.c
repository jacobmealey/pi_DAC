// A. Sheaff 3/15/2019 - R-2R Ladder DAC driver
// Framework code for creating a kernel driver
// that creates an analog voltage based on 8 bit
// from 8 gpio pins
// Pass data in by write()
// Set frequency, start, stop by ioctl()
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio/consumer.h>
#include <linux/jiffies.h>
#include <linux/stat.h>

#include "dac.h"

// Data to be "passed" around to various functions
struct dac_data_t {
	struct gpio_desc *gpio_dac_b0;		// DAC Bit 0 pin
	struct gpio_desc *gpio_dac_b1;		// DAC Bit 1 pin
	struct gpio_desc *gpio_dac_b2;		// DAC Bit 2 pin
	struct gpio_desc *gpio_dac_b3;		// DAC Bit 3 pin
	struct gpio_desc *gpio_dac_b4;		// DAC Bit 4 pin
	struct gpio_desc *gpio_dac_b5;		// DAC Bit 5 pin
	struct gpio_desc *gpio_dac_b6;		// DAC Bit 6 pin
	struct gpio_desc *gpio_dac_b7;		// DAC Bit 7 pin
	struct class *dac_class;			// Class for auto /dev population
	struct device *dac_dev;				// Device for auto /dev population
	struct platform_device *pdev;		// Platform driver
	int dac_major;						// Device major number

	u32 dac_enable;						// DAC Enable
	u32 dac_freq;						// DAC Frequency in Hz

	// Locking variable appears after this line
};

// DAC data structure access between functions
static struct dac_data_t *dac_dat=NULL;

// ioctl system call
//   For enable, set the enable value to true
//   For disable, set the enable value to false
//   For setting the sample frequency, save the 'arg' argument to the sample frequency variable
static long dac_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
{
	long ret=0;					// Return value


	// IOCTL cmds
	switch (cmd) {
		default:
			ret=-EINVAL;
			break;
	}

	return ret;
}

// Write system call
//   If the DAC is disabled, return an appropriate error
//   Allocate kernel memory to hold the user data passed in buf.
//   Copy the user data to the buffer just allocated in kernel space
//   Calculate the sample period ****  NO FLOATING POINT CALCULATIONS ALLOWED ****
//   Write the data, a bit at a time, to the GPIO pins.
//   Free allocated resources.
// As always, act appropriately upon error.
// Use propriate types
static ssize_t dac_write(struct file *filp, const char __user * buf, size_t count, loff_t * offp)
{

	uint8_t i;
	size_t data_len = 30;
	size_t num_copied;
	uint8_t data[30];
	if(count == 0){
		return 0;
	}

	if(count < data_len){
		data_len = count;
	}

	num_copied = copy_from_user(data, buf, data_len);

	if(num_copied == 0){
		printk("dac_write: Copied %d bytes", data_len);
	} else {
		printk("dac_write: Copied %d bytes", num_copied);
	}
	
	data[data_len] = 0;
	// ----- Beginning of writing to pins ----- //
	for(i = 0; i < data_len; i++){
		gpiod_set_value(dac_dat->gpio_dac_b7, (data[i] >> 7) & 0x1);
		gpiod_set_value(dac_dat->gpio_dac_b6, (data[i] >> 6) & 0x1);
		gpiod_set_value(dac_dat->gpio_dac_b5, (data[i] >> 5) & 0x1);
		gpiod_set_value(dac_dat->gpio_dac_b4, (data[i] >> 4) & 0x1);
		gpiod_set_value(dac_dat->gpio_dac_b3, (data[i] >> 3) & 0x1);
		gpiod_set_value(dac_dat->gpio_dac_b2, (data[i] >> 2) & 0x1);
		gpiod_set_value(dac_dat->gpio_dac_b1, (data[i] >> 1) & 0x1);
		gpiod_set_value(dac_dat->gpio_dac_b0, (data[i]) & 0x1);
		// this will need to a better calculated value maybe usleep
		msleep(500);
		printk("Copied %zd from userspace", data[i]);
	}

	return count;
}

// You will need to choose the type of locking yourself.  It may be atmonic variables, spinlocks, mutex, or semaphore.

// Open system call
// Open only if the file access flags (NOT permissions) are appropiate as discussed in class
// Return an appropraite error otherwise
//   If the file is opened non-blocking and the lock is held,
//     return with an appropraite error
//   else
//     If the lock is held,
//       Wait until the lock is released and then acquire the lock
//     else
//       Acquire the lock
static int dac_open(struct inode *inode, struct file *filp)
{

	printk(KERN_INFO "dac opened");
	return 0;
}

// Close system call
// Release the lock
static int dac_release(struct inode *inode, struct file *filp)
{

    printk(KERN_INFO "dac released");
    return 0;
}

// File operations for the dac device.  Uninitialized will be NULL.
static const struct file_operations dac_fops = {
	.owner = THIS_MODULE,	// Us
	.open = dac_open,		// Open
	.release = dac_release,// Close
	.write = dac_write,	// Write
	.unlocked_ioctl=dac_ioctl,	// ioctl
};

// Allocate our GPIO pins from the kernel
// Init value<0 means input
static struct gpio_desc *dac_obtain_pin(struct device *dev, int pin, char *name, int init_val)
{
	struct gpio_desc *gpiod_pin=NULL;	// GPIO Descriptor for setting value
	int ret=-1;	// Return value

	// Request the pin - release with devm_gpio_free() by pin number
	if (init_val>=0) {
		ret=devm_gpio_request_one(dev,pin,GPIOF_OUT_INIT_LOW,name);
		if (ret<0) {
			dev_err(dev,"Cannot get %s gpio pin\n",name);
			gpiod_pin=NULL;
			goto fail;
		}
	} else {
		ret=devm_gpio_request_one(dev,pin,GPIOF_IN,name);
		if (ret<0) {
			dev_err(dev,"Cannot get %s gpio pin\n",name);
			gpiod_pin=NULL;
			goto fail;
		}
	}

	// Get the gpiod pin struct
	gpiod_pin=gpio_to_desc(pin);
	if (gpiod_pin==NULL) {
		printk(KERN_INFO "Failed to acquire dac gpio\n");
		gpiod_pin=NULL;
		goto fail;
	}

	// Make sure the pin is set correctly
	if (init_val>=0) gpiod_set_value(gpiod_pin,init_val);

	return gpiod_pin;

fail:
	if (ret>=0) devm_gpio_free(dev,pin);

	return gpiod_pin;
}


// Sets device node permission on the /dev device special file
static char *dac_devnode(struct device *dev, umode_t *mode)
{
	if (mode) *mode = S_IRUGO|S_IWUGO;
	return NULL;
}

// This is called on module load.
static int __init dac_probe(void)
{
	int ret=-1;	// Return value

	// Allocate device driver data and save
	dac_dat=kmalloc(sizeof(struct dac_data_t),GFP_KERNEL);
	if (dac_dat==NULL) {
		printk(KERN_INFO "Memory allocation failed\n");
		return -ENOMEM;
	}

	memset(dac_dat,0,sizeof(struct dac_data_t));

	// Create the device - automagically assign a major number
	dac_dat->dac_major=register_chrdev(0,"dac",&dac_fops);
	if (dac_dat->dac_major<0) {
		printk(KERN_INFO "Failed to register character device\n");
		ret=dac_dat->dac_major;
		goto fail;
	}

	// Create a class instance
	dac_dat->dac_class=class_create(THIS_MODULE, "dac_class");
	if (IS_ERR(dac_dat->dac_class)) {
		printk(KERN_INFO "Failed to create class\n");
		ret=PTR_ERR(dac_dat->dac_class);
		goto fail;
	}

	// Setup the device so the device special file is created with 0666 perms
	dac_dat->dac_class->devnode=dac_devnode;
	dac_dat->dac_dev=device_create(dac_dat->dac_class,NULL,MKDEV(dac_dat->dac_major,0),(void *)dac_dat,"dac");
	if (IS_ERR(dac_dat->dac_dev)) {
		printk(KERN_INFO "Failed to create device file\n");
		ret=PTR_ERR(dac_dat->dac_dev);
		goto fail;
	}

	dac_dat->gpio_dac_b0=dac_obtain_pin(dac_dat->dac_dev,4,"DAC_b0",0);
	if (dac_dat->gpio_dac_b0==NULL) goto fail;
	dac_dat->gpio_dac_b1=dac_obtain_pin(dac_dat->dac_dev,17,"DAC_b1",0);
	if (dac_dat->gpio_dac_b1==NULL) goto fail;
	dac_dat->gpio_dac_b2=dac_obtain_pin(dac_dat->dac_dev,18,"DAC_b2",0);
	if (dac_dat->gpio_dac_b2==NULL) goto fail;
	dac_dat->gpio_dac_b3=dac_obtain_pin(dac_dat->dac_dev,27,"DAC_b3",0);
	if (dac_dat->gpio_dac_b3==NULL) goto fail;
	dac_dat->gpio_dac_b4=dac_obtain_pin(dac_dat->dac_dev,22,"DAC_b4",0);
	if (dac_dat->gpio_dac_b4==NULL) goto fail;
	dac_dat->gpio_dac_b5=dac_obtain_pin(dac_dat->dac_dev,23,"DAC_b5",0);
	if (dac_dat->gpio_dac_b5==NULL) goto fail;
	dac_dat->gpio_dac_b6=dac_obtain_pin(dac_dat->dac_dev,24,"DAC_b6",0);
	if (dac_dat->gpio_dac_b6==NULL) goto fail;
	dac_dat->gpio_dac_b7=dac_obtain_pin(dac_dat->dac_dev,25,"DAC_b7",0);
	if (dac_dat->gpio_dac_b7==NULL) goto fail;

	// Initialize locking below this line


	printk(KERN_INFO "Registered\n");

	return 0;

fail:
	if (dac_dat->gpio_dac_b7) devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b7));
	if (dac_dat->gpio_dac_b6) devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b6));
	if (dac_dat->gpio_dac_b5) devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b5));
	if (dac_dat->gpio_dac_b4) devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b4));
	if (dac_dat->gpio_dac_b3) devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b3));
	if (dac_dat->gpio_dac_b2) devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b2));
	if (dac_dat->gpio_dac_b1) devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b1));
	if (dac_dat->gpio_dac_b0) devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b0));

	// Device cleanup
	if (dac_dat->dac_dev) device_destroy(dac_dat->dac_class,MKDEV(dac_dat->dac_major,0));
	// Class cleanup
	if (dac_dat->dac_class) class_destroy(dac_dat->dac_class);
	// char dev clean up
	if (dac_dat->dac_major>0) unregister_chrdev(dac_dat->dac_major,"dac");

	kfree(dac_dat);
	printk(KERN_INFO "DAC Failed\n");
	return ret;
}

// Called when the module is removed.
static void __exit dac_remove(void)
{
	// Free the gpio pins with devm_gpio_free() & gpiod_put()
	devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b7));
	devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b6));
	devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b5));
	devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b4));
	devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b3));
	devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b2));
	devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b1));
	devm_gpio_free(dac_dat->dac_dev,desc_to_gpio(dac_dat->gpio_dac_b0));

	// Device cleanup
	if (dac_dat->dac_dev) device_destroy(dac_dat->dac_class,MKDEV(dac_dat->dac_major,0));
	// Class cleanup
	if (dac_dat->dac_class) class_destroy(dac_dat->dac_class);
	// Remove char dev
	if (dac_dat->dac_major>0) unregister_chrdev(dac_dat->dac_major,"dac");
	
	// Free the device driver data
	if (dac_dat!=NULL) {
		kfree(dac_dat);
		dac_dat=NULL;
	}

	printk(KERN_INFO "Removed\n");
}

module_init(dac_probe);
module_exit(dac_remove);

MODULE_DESCRIPTION("RPI R-2R DAC");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DAC");
