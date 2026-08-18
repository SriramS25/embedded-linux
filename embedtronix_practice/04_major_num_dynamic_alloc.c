#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/printk.h>

dev_t dev = 0;

static int __init hello_init(void){
	int ret = alloc_chrdev_region(&dev,0,1, " Sample CHARDEV Dynamic alloc - Major number");
	
	if(ret < 0){
		pr_info("Cannot allocate major number for this device \n");
		return ret;
	}

	pr_info("Major = %d    Minor = %d\n",MAJOR(dev),MINOR(dev));
	pr_info("Kernel Module inserted Successfully\n");
	return 0;
}

static void __exit hello_exit(void){
	unregister_chrdev_region(dev,1);
	pr_info("Kernel Module removed Successfully..\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sriram S");
MODULE_DESCRIPTION("Dynamically allocating MAJOR number");
MODULE_VERSION("1.0");

