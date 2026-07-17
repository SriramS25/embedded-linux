#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/printk.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "mydev_ioctl.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("WOLF");
MODULE_DESCRIPTION("Character Device Driver - Phase 4 IOCTL");
MODULE_VERSION("1.0");

/*---Kernel Buffer--*/
#define BUFFER_SIZE_DEFAULT 1024
static char *kernel_buf = NULL;
static int buf_len = 0;  //to track how many bytes currently in buffer
static int buf_size = BUFFER_SIZE_DEFAULT;

//--- stats --//
static int read_count = 0;
static int write_count = 0;

/* -- globals --*/
static dev_t 	dev_num;   		//holds major + minor number
static struct 	cdev my_cdev;		//kernel's char device structure
static struct 	class *my_class;		//device class - appears in /sys/class/
static struct 	device *my_device; 	//device node - triggers udev -> /dev/mydev

/*====-- file operations --====*/
static int my_open(struct inode *inode, struct file *file){
 	dev_info(my_device, "=== open() called ===\n");
    	dev_info(my_device, "  inode->i_rdev  = %d:%d (major:minor)\n",
             MAJOR(inode->i_rdev), MINOR(inode->i_rdev));
    	dev_info(my_device, "  inode->i_ino   = %lu (inode number)\n",
             inode->i_ino);
    	dev_info(my_device, "  file->f_flags  = 0x%x (O_RDONLY=0 O_WRONLY=1 O_RDWR=2)\n",
             file->f_flags);
    	dev_info(my_device, "  file->f_mode   = 0x%x\n",
             file->f_mode);
    	dev_info(my_device, "  file->f_pos    = %lld (current position)\n",
             file->f_pos);	
	return 0;
}

static int my_release(struct inode *inode, struct file *file){
	dev_info(my_device, "=== close() called ===\n");
    dev_info(my_device, "  file->f_pos at close = %lld\n",
             file->f_pos);
    dev_info(my_device, "  total bytes accessed = %lld\n",
             file->f_pos);
	return 0;
}

static ssize_t my_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos){
	int bytes_to_read;
	read_count++;

	dev_info(my_device, "=== read() called [#%d] ===\n",read_count);
    	dev_info(my_device, "  ubuf  (userspace ptr) = %p\n",  ubuf);
    	dev_info(my_device, "  count (requested)     = %zu bytes\n", count);
	dev_info(my_device, "  *ppos (file offset)   = %lld\n", *ppos);
    	dev_info(my_device, "  buf_len (data in buf) = %d bytes\n", buf_len);
    	dev_info(my_device, "  kernel_buf content    = [%s]\n", kernel_buf);
	
	//Nothing left to read
	if(*ppos >= buf_len){
		 dev_info(my_device, "  ppos(%lld) >= buf_len(%d) → returning EOF\n",
                 *ppos, buf_len);
		 dev_info(my_device, "EOF -- returning 0\n");
		return 0;
	}	


	//Dont read more that whats available
	bytes_to_read = min((size_t)(buf_len - *ppos),count);
	 dev_info(my_device, "  bytes_to_read = min(%zu, %zu) = %d\n",
             (size_t)(buf_len - *ppos), count, bytes_to_read);

	//copy from kernel buffer -> userspace
	 dev_info(my_device, "  copy_to_user: kernel[%p] → user[%p], %d bytes\n",
             kernel_buf + *ppos, ubuf, bytes_to_read);
	if(copy_to_user(ubuf,kernel_buf + *ppos, bytes_to_read)){
		dev_err(my_device,"copy_to_user failed!\n");
		return -EFAULT;
	}

	*ppos += bytes_to_read;
	   dev_info(my_device, "  *ppos updated to = %lld\n", *ppos);
    dev_info(my_device, "  returning %d to userspace\n", bytes_to_read);
	return bytes_to_read;
}

static ssize_t my_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos){
	int bytes_to_write;
	write_count++;
	dev_info(my_device, "=== write() called [#%d] ===\n",write_count);
    	dev_info(my_device, "  ubuf  (userspace ptr) = %p\n",  ubuf);
    	dev_info(my_device, "  count (bytes sent)    = %zu bytes\n", count);
    	dev_info(my_device, "  *ppos (file offset)   = %lld\n", *ppos);
    	dev_info(my_device, "  buf_size	             = %d\n",   buf_size);
    	dev_info(my_device, "  buf_len before write  = %d\n",   buf_len);

	//dont overflow kernel buffer
	bytes_to_write = min(count,(size_t)(buf_size - 1));
	dev_info(my_device, "  bytes_to_write = min(%zu, %d) = %d\n",
             count, buf_size - 1, bytes_to_write);

	//copy from userspace -> kenrel buffer
	  dev_info(my_device, "  copy_from_user: user[%p] → kernel[%p], %d bytes\n",
             ubuf, kernel_buf, bytes_to_write);
	if(copy_from_user(kernel_buf, ubuf, bytes_to_write)){
		dev_err(my_device, "copy_from_user failed\n");
		return -EFAULT;
	}

	kernel_buf[bytes_to_write] = '\0';
	buf_len = bytes_to_write;
	*ppos += bytes_to_write;

	dev_info(my_device, "  kernel_buf now holds  = [%s]\n", kernel_buf);
    dev_info(my_device, "  buf_len updated to    = %d\n",   buf_len);
    dev_info(my_device, "  *ppos updated to      = %lld\n", *ppos);
    dev_info(my_device, "  returning %d to userspace\n",    bytes_to_write);
	return bytes_to_write;
}

//==========IOCTL
static long my_ioctl(struct file *file, unsigned int cmd,  unsigned long arg){
	int ret = 0;
	int val = 0;
	mydev_stats_t stats;
	char *new_buf = NULL;

	dev_info(my_device,"==ioctl() called ==\n");
	dev_info(my_device," cmd = 0x%x\n", cmd);
	dev_info(my_device," arg = %lu\n", arg);

	//Verify command belogs to our driver
	if(_IOC_TYPE(cmd) != MYDEV_MAGIC){
		dev_err(my_device, "wrong magic number\n");
		return -ENOTTY;
	}

	switch(cmd){
		//--CMD 0: Clear Buffer
		case MYDEV_CLEAR:
			dev_info(my_device, "CMD: MYDEV_CLEAR\n");
			memset(kernel_buf, 0 ,buf_size);
			buf_len = 0;
			dev_info(my_device, "buffer cleared\n");
			break;

		//CMD 1: Get buffer length 
		case MYDEV_GET_LEN:
			dev_info(my_device, "CMD: MYDEV_GET_LEN\n");
			dev_info(my_device, "buf_len = %d -> sending to userspace\n",buf_len);
			if(copy_to_user((int __user *)arg, &buf_len, sizeof(int))){
				dev_err(my_device, " copy_to_user failed\n");
				return -EFAULT;
			}
			break;

		//CMD 2: Set buffer size
		case MYDEV_SET_SIZE:
			dev_info(my_device," CMD: MYDEV_SET_SIZE\n");
		if(copy_from_user(&val, (int __user *)arg, sizeof(int))){
			dev_err(my_device, "copy_from_user failed\n");

				return -EFAULT;
			}

		dev_info(my_device," requested new size = %d\n",val);

		//validate size
		if(val <=0 || val > 65536){
			dev_err(my_device, "invalid size\n");
			return -EINVAL;
		}

		//Allocate new buffer
		new_buf = kmalloc(val, GFP_KERNEL);
		if(!new_buf){
			dev_err(my_device, "kmalloc failed\n");
			return -ENOMEM;
		}

		//Free old, install new
		kfree(kernel_buf);
		kernel_buf = new_buf;
		buf_size = val;
		buf_len = 0;
		memset(kernel_buf, 0 , buf_size);

		dev_info(my_device, " buffer resized to %d bytes \n", buf_size);
		break;

		//---CMD 3 : Get stats
		case MYDEV_GET_STATS:
			dev_info(my_device, " CMD: MYDEV_GET_STATS\n");
			stats.buf_size = buf_size;
			stats.buf_len = buf_len;
			stats.read_count= read_count;
			stats.write_count = write_count;

			dev_info(my_device, "stats: size=%d len=%d reads=%d writes=%d\n",stats.buf_size,stats.buf_len,stats.read_count, stats.write_count);
			if(copy_to_user((mydev_stats_t __user *)arg,&stats, sizeof(mydev_stats_t))){
				dev_err(my_device,"copy_to_user failed\n");
				return -EFAULT;
			}
			break;

		default:
			dev_err(my_device, "UNKNOWN command\n");
			return -ENOTTY;
	}
	return ret;
}

//=====fops table==============

static struct file_operations fops={
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_release,
	.read = my_read,
	.write = my_write,
	.unlocked_ioctl = my_ioctl,
};

/*--- init----*/
static int __init mydev_init(void)
{
	int ret;

	//Allocate kern buff dynamically
	kernel_buf = kmalloc(BUFFER_SIZE_DEFAULT, GFP_KERNEL);
	if(!kernel_buf){
		pr_err("mydev : kmalloc failed\n");
		return -ENOMEM;
	}

	memset(kernel_buf, 0, BUFFER_SIZE_DEFAULT);
	pr_info("mydev: kernel_buf allocated %d bytes\n", BUFFER_SIZE_DEFAULT);
	
	//step 1: dynamic major/ minor allocation
	ret = alloc_chrdev_region(&dev_num, 0 ,1,"mydev");
	if(ret < 0)
	{
		pr_err("mydev: alloc_chrdev failed to allocate major/minor\n");
		goto err_alloc;
	}
	pr_info("mydev: allocated major = %d  minor = %d\n",MAJOR(dev_num), MINOR(dev_num));

	//step 2: init cdev & link fops
	cdev_init(&my_cdev,&fops);
	my_cdev.owner = THIS_MODULE;
	
	//step 3: add cdev to kernel
	ret = cdev_add(&my_cdev, dev_num, 1);
	if(ret < 0){
		pr_err("mydev: cdev_Add failed \n");
		goto err_cdev;
	}
	
	//step 4: Create Class
	my_class = class_create("mydev");
	if(IS_ERR(my_class)){
		pr_err("mydev: class_create failed\n");
		ret = PTR_ERR(my_class);
		goto err_class;
	}	
	

	//step 5: create device node -> /dev/mydev
	my_device = device_create(my_class, NULL, dev_num, NULL, "mydev");
	if(IS_ERR(my_device)){
		pr_err("mydev: device_create failed");
		ret = PTR_ERR(my_device);
		goto err_device;		
		}
	
	dev_info(my_device, "mydev ready at /dev/mydev \n");
	return 0;

	//err unwiding using goto 
err_device:
	class_destroy(my_class);
err_class:
	cdev_del(&my_cdev);
err_cdev:
	unregister_chrdev_region(dev_num, 1);
err_alloc:
	kfree(kernel_buf);
	return ret;
}



static void __exit mydev_exit(void)
{	
	device_destroy(my_class,dev_num);
	class_destroy(my_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(dev_num, 1);
	kfree(kernel_buf);
	printk(KERN_INFO "mydev: module removed\n");
} 

module_init(mydev_init);
module_exit(mydev_exit);
