//--------------------------INCLUDE HEADER FILES---------------------------------
#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/types.h>
#include<linux/cdev.h>
#include<linux/fs.h> //for various structures related to files(e.g.fops)
#include<linux/device.h>
#include<asm/uaccess.h> //for copy_from_user and copy_to user functions
#include<linux/moduleparam.h>
#include <linux/ioctl.h> //for ioctl
#include <linux/version.h> //for LINUX_VERSION_CODE and KERNEL_VERSION Macros
#include <linux/errno.h> //Linux system errors
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/gpio.h>    
#include <linux/pci.h>

//--------------------------DECLARATIONS AND DEFINITIONS---------------------------------
#define DEVICE_NAME "delayLED"     //the device will appear as rbt530_dev in /dev/ when insmod is done
#define DEVICE_CLASS "delayLED_class"     //the device class will appear as rbt530_class in /sys/class
#define DEVICES 1   

static dev_t device_no; //major and minor number calculated using this
unsigned int MAJOR_NUMBER = 77, MINOR_NUMBER = 0;
static int    open_count = 0;              ///< Counts the number of times the device is opened
static int is_device_open = 0;		//to check if device is already open
static struct cdev cdev_info;
static struct class *class_data;
static char *user_name = "TEAM 19";
int my_global_variable = 99;
module_param(user_name,charp,0000); //Passing command line arguments to a module; module_param(variable, type, permissions)

//-------------- Function Declarations ------------------------------------------

static int __init rbt530_dev_init(void); //init function
static void __exit rbt530_dev_exit(void); //exit function

static int rbt530_dev_open(struct inode *, struct file *); //open
static int rbt530_dev_release(struct inode *, struct file *); //release

static ssize_t rbt530_dev_write(struct file *, const char *, size_t, loff_t *); //write

typedef struct LEDdata {   
	char LEDhexData[16*6];
	} LED_data;


//------------------------ File_operations structure-------------------------------
static struct file_operations fops =
{ 
	.owner = THIS_MODULE,
      	.open = rbt530_dev_open, //open
        .release = rbt530_dev_release,//release or close
        .write = rbt530_dev_write,//write
    };


//------------------------------- Function definitions -----------------------------------


static int __init rbt530_dev_init(void) //Init function called when insmod is done
{
	int result;

	// Device Registeration
	result =1;
    	result=alloc_chrdev_region(&device_no,0,DEVICES,DEVICE_NAME);
   	if(!result)
   	{
    		MAJOR_NUMBER=MAJOR(device_no);
    		MINOR_NUMBER=MINOR(device_no);
    		printk(KERN_INFO "team 19: Device Registration complete with Major,Minor pair (%d,%d)\n",MAJOR_NUMBER,MINOR_NUMBER);
    		
    	}
    	else
    	{
        	goto RETURN_ERROR; //ERROR; cant register
    	}
    	printk(KERN_DEBUG "Device registered with the kernel\n");
    	


	//Class Creation
    	if((class_data=class_create(THIS_MODULE,DEVICE_CLASS))==NULL)
    	{
    		goto UNREGISTER;
    	}
    	printk(KERN_DEBUG "Class created\n");
    

	//Device Creation
    	if(device_create(class_data,NULL,device_no,NULL,DEVICE_NAME)==NULL)
    	{
    		goto CLASS_DESTROY;
    	}
    	printk(KERN_DEBUG "Device created\n");
    	

	//Device file creation:
    	cdev_init(&cdev_info,&fops);
    	if(cdev_add(&cdev_info,device_no,DEVICES)<0)
    	{
    		goto DEVICE_DESTROY;
    	}
    	printk(KERN_DEBUG "Device file created\n");
    	return 0;

	//----------------------- GOTO LABELS for error handling----------------------------
    	DEVICE_DESTROY: device_destroy(class_data,device_no);
    	CLASS_DESTROY: class_destroy(class_data);
    	UNREGISTER: unregister_chrdev_region(device_no,DEVICES);
    	RETURN_ERROR: return -1;
}

static void __exit rbt530_dev_exit(void) //Exit function when rmmod is done
{
    	cdev_del(&cdev_info); 
    	device_destroy(class_data,device_no);
    	class_destroy(class_data);
    	unregister_chrdev_region(device_no,DEVICES);
    	printk(KERN_ALERT "Goodbye user!! %s\n",user_name);
    	return;
}


static int rbt530_dev_open(struct inode *node, struct file *filp) //open
{

	if(is_device_open)
	{
		printk(KERN_INFO "team 19: The device is already open!!\n");
		return -1; /*We don't want to talk to two processes at the  same time */
	}
	is_device_open++;
	open_count++;
   	printk(KERN_INFO "team 19: The driver has been opened %d times\n", open_count);
	return 0;
}

static int rbt530_dev_release(struct inode *node, struct file *filp) //release (when close(fd) is called)
{   
   
	if (!is_device_open)
	{
		printk(KERN_INFO "team 19: The device is not open!!\n");
		return -1;
	}
	is_device_open--;
	gpio_free(12);
	printk(KERN_INFO "team 19: The driver has been released\n");
	return 0;
}


static ssize_t rbt530_dev_write(struct file *filp, const char *buff, size_t length,loff_t *off) //write to device
{
	struct pci_dev *pdev = pci_get_device(0x8086, 0x0934, NULL);	
	resource_size_t start = 0, len = 0;
	void __iomem *reg_base;
	void __iomem *reg_data;
	u32 val_data = 0;
	int ret,i,j;
	int numberOfLeds,numberOfBits;
	char dataBinary[16*24]="";	
	LED_data *dataToLED = kmalloc(sizeof(LED_data), GFP_KERNEL); 
	ret = copy_from_user(dataToLED, buff, sizeof(LED_data));	//get the hex LED pixel data from user space
	if (ret!=0)
		printk(KERN_INFO"Error copying from user!!!\n");
	
	start = pci_resource_start(pdev, 1);
	len = pci_resource_len(pdev, 1);
	
	reg_base = ioremap_nocache(start, len);	

	reg_data = reg_base + 0x00;

	val_data = ioread32(reg_data);

	for(j=0;j<6*length;j++)	//each hex data is converted to the binary represenation
	{
		switch(dataToLED->LEDhexData[j])
		{
			case '0': strcat(dataBinary,"0000");
					break;
			case '1': strcat(dataBinary,"0001");
					break;
			case '2': strcat(dataBinary,"0010");
					break;
			case '3': strcat(dataBinary,"0011");
					break;
			case '4': strcat(dataBinary,"0100");
					break;
			case '5': strcat(dataBinary,"0101");
					break;
			case '6': strcat(dataBinary,"0110");
					break;
			case '7': strcat(dataBinary,"0111");
					break;
			case '8': strcat(dataBinary,"1000");
					break;
			case '9': strcat(dataBinary,"1001");
					break;
			case 'a'|'A': strcat(dataBinary,"1010");
					break;
			case 'b'|'B': strcat(dataBinary,"1011");
					break;
			case 'c'|'C': strcat(dataBinary,"1100");
					break;
			case 'd'|'D': strcat(dataBinary,"1101");
					break;
			case 'e'|'E': strcat(dataBinary,"1110");
					break;
			case 'f'|'F': strcat(dataBinary,"1111");
					break;
			default: return -1;
		}
	}
	kfree(dataToLED);
	printk(KERN_INFO "team 19: Delay LED Write accessed\n");
	//setting gpio12 (io1)
	gpio_request(12, "trigger_pin");
	gpio_direction_output(28, 0);
	gpio_request(45, "MUX1");
	gpio_set_value(45,0x00);
	gpio_export(12, 0);
	
	numberOfLeds = length;	//number of LEDs
	numberOfBits = numberOfLeds * 24;	//total number of binary bits
	//initially setting a value of 0
	iowrite32(val_data & ~BIT(4 % 32), reg_data);
	ndelay(500);
	for(i=0;i<numberOfBits;i++)	//iterate over all bits
	{
		if (dataBinary[i] == '1')	//if 1 create the corresponding signal with longer high pulse
		{
			iowrite32(val_data | BIT(4 % 32), reg_data);	//setting high
			ndelay(450);
			iowrite32(val_data & ~BIT(4 % 32), reg_data);	//setting low
			ndelay(250);
		}
		else	//if 0 create the corresponding signal with longer low pulse
		{
			iowrite32(val_data | BIT(4 % 32), reg_data);	//setting high
			ndelay(100);
			iowrite32(val_data & ~BIT(4 % 32), reg_data);	//setting low
			ndelay(300);
		}
	}
	
	return 0;
}
//____________________________________________________________________________________


module_init( rbt530_dev_init);
module_exit( rbt530_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Team 19");
MODULE_DESCRIPTION("Accessing a kernel RB tree via device file interface");
