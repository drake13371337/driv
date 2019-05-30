#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "conv_d"
#define CLASS_NAME  "conv_c"
#define MES_SIZE 256
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)
#define WR_VALUE_2 _IOW('a','c',int32_t*)
#define RD_VALUE_2 _IOR('a','d',int32_t*)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mikhail Karpov");
MODULE_DESCRIPTION("Convertor");
MODULE_VERSION("0.1");

static char message[MES_SIZE] = {0};
static int size;
static int numberOpens = 0;
static int majorNumber;
static struct class* conv_Class  = NULL;
static struct device* conv_Device = NULL;

static int  __init conv_init(void);
static void __exit conv_exit(void);

static int dev_open(struct inode*, struct file*);
static int dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);
static long dev_ioctl(struct file*, unsigned int, unsigned long);

static int32_t input_encoding = 0;
static int32_t output_encoding = 0;

static unsigned *current_encoding;
static unsigned *new_encoding;

static const unsigned cp1251[] = {192, 193, 194, 195, 196, 197, 168, 198, 199,
                                  200, 201, 202, 203, 204, 205, 206, 207, 208,
                                  209, 210, 211, 212, 213, 214, 215, 216, 217,
                                  218, 219, 220, 221, 222, 223, 224, 225, 226,
                                  227, 228, 229, 184, 230, 231, 232, 233, 234,
                                  235, 236, 237, 238, 239, 240, 241, 242, 243,
                                  244, 245, 246, 247, 248, 249, 250, 251, 252,
                                  253, 254, 255};

static const unsigned koi8_r[] = {225, 226, 247, 231, 228, 229, 179, 246, 250,
                                  233, 234, 235, 236, 237, 238, 239, 240, 242,
                                  243, 244, 245, 230, 232, 227, 254, 251, 253,
                                  255, 249, 248, 252, 224, 241, 193, 194, 215,
                                  199, 196, 197, 163, 214, 218, 201, 202, 203,
                                  204, 205, 206, 207, 208, 210, 211, 212, 213,
                                  198, 200, 195, 222, 219, 221, 223, 217, 216,
                                  220, 192, 209};

static const unsigned cp866[]  = {128, 129, 130, 131, 132, 133, 240, 134, 135,
                                  136, 137, 138, 139, 140, 141, 142, 143, 144,
                                  145, 146, 147, 148, 149, 150, 151, 152, 153,
                                  154, 155, 156, 157, 158, 159, 160, 161, 162,
                                  163, 164, 165, 241, 166, 167, 168, 169, 170,
                                  171, 172, 173, 174, 175, 224, 225, 226, 227,
                                  228, 229, 230, 231, 232, 233, 234, 235, 236,
                                  237, 238, 239};


static int rec(char *buffer, const unsigned *c_in, const unsigned *c_out){
	unsigned long i, j;

	for(i = 0; i < size; ++i){
		unsigned char ch = (unsigned char)buffer[i];
		for(j = 0; j < 66; ++j){
			if(ch == c_in[j]){
				buffer[i] = (char)c_out[j];
				break;
			}
		}
	}
	printk(KERN_INFO "conv_mod: Rec complite");
	return 0;
}

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
   .unlocked_ioctl = dev_ioctl
};

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	switch (cmd){
		case WR_VALUE:
			copy_from_user(&input_encoding ,(int32_t*) arg, sizeof(input_encoding));
			if(input_encoding == 1){
				current_encoding = cp866;
				printk(KERN_INFO "conv_mod: set cp866");
			}
			if(input_encoding == 2){
				current_encoding = koi8_r;
				printk(KERN_INFO "conv_mod: set KOI8-R");
			}
			if(input_encoding == 3){
				current_encoding = cp1251;
				printk(KERN_INFO "conv_mod: set cp1251");
			}
			break;
		case RD_VALUE:
			copy_to_user((int32_t*) arg, &input_encoding, sizeof(input_encoding));
			break;
		case WR_VALUE_2:
			copy_from_user(&output_encoding ,(int32_t*) arg, sizeof(output_encoding));
			if(output_encoding == 1)
				new_encoding = cp866;
			if(output_encoding == 2)
				new_encoding = koi8_r;
			if(output_encoding == 3)
				new_encoding = cp1251;
			break;
		case RD_VALUE_2:
			copy_to_user((int32_t*) arg, &output_encoding, sizeof(output_encoding));
			break;
    }
    return 0;
}

static int __init conv_init(void){
	printk(KERN_INFO "conv_mod: Initializing the Char device\n");

	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if(majorNumber<0){
		printk(KERN_ALERT "Char device failed to register a major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "conv_mod: Registered correctly with major number %d\n", majorNumber);

	conv_Class = class_create(THIS_MODULE, CLASS_NAME);
	if(IS_ERR(conv_Class)){
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(conv_Class);
	}
	printk(KERN_INFO "conv_mod: Device class registered correctly\n");

	conv_Device = device_create(conv_Class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if(IS_ERR(conv_Device)){
		class_destroy(conv_Class);
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(conv_Device);
	}
	printk(KERN_INFO "conv_mod: Device class created correctly\n");
	return 0;
}

static void __exit conv_exit(void){
	device_destroy(conv_Class, MKDEV(majorNumber, 0));
	class_unregister(conv_Class);
	class_destroy(conv_Class);
	unregister_chrdev(majorNumber, DEVICE_NAME);
	printk(KERN_INFO "conv_mod: Module has been unloaded!\n");
}

static int dev_open(struct inode *inodep, struct file *filep){
	numberOpens++;
	printk(KERN_INFO "conv_mod: Device has been opened %d time(s)\n", numberOpens);
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
	int error_count = 0;

	rec(message,current_encoding,new_encoding);
	error_count = copy_to_user(buffer, message, size);

	if(error_count==0){
		printk(KERN_INFO "conv_mod: Sent %d characters to the user\n", size);
		return (size=0);
	} else {
		printk(KERN_INFO "conv_mod: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;
	}
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
	int res;
	if(len>MES_SIZE) size=MES_SIZE;
	else size=len;
	res=copy_from_user(message, buffer, len);
	printk(KERN_INFO "conv_mod: Received %zu characters from the user\n", len);
	return len;
}

static int dev_release(struct inode *inodep, struct file *filep){
	printk(KERN_INFO "conv_mod: Device successfully closed\n");
	return 0;
}

module_init(conv_init);
module_exit(conv_exit);
