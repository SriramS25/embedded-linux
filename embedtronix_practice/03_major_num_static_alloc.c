#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/printk.h>

//Creating the dev with our custom major and minor number
dev_t dev = MKDEV(235,0);


static int __init hello_init(void){
	int ret = register_chrdev_region(dev, 1, "Sample CHARDEV - static allocation - major number");
	if (ret < 0) {
	         pr_err("Failed to register device: %d\n", ret);
       		 return ret;
        }
	pr_info("Major = %d\n Minor = %d\n", MAJOR(dev), MINOR(dev));
	pr_info("Kernel Module Inserted Successfully!\n");
	return 0;
}

static void __exit hello_exit(void){
	unregister_chrdev_region(dev,1);
	pr_info("Kernel Module Removed Succesfully\n");
}

module_init(hello_init);
module_exit(hello_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sriram S");
MODULE_DESCRIPTION("Statically allocating MAJOR number");
MODULE_VERSION("1.0");
