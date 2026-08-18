#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/device.h>

dev_t dev = 0;

static struct class *dev_class;
static struct device *dev_device;


static int __init hello_init(void)
{
    int ret;

    pr_info("[INIT] Module initialization started\n");


    /*---------------------------------------------------------*/
    /* Step 1: Allocate major/minor number                     */
    /*---------------------------------------------------------*/

    pr_info("[INIT] Calling alloc_chrdev_region()\n");

    ret = alloc_chrdev_region(&dev, 0, 1, "my_Dev");

    if (ret < 0) {
        pr_err("[ERROR] alloc_chrdev_region() failed\n");
        pr_err("[ERROR] ret = %d\n", ret);
        return ret;
    }

    pr_info("[INIT] Device number allocated successfully\n");
    pr_info("[INIT] Major = %d\n", MAJOR(dev));
    pr_info("[INIT] Minor = %d\n", MINOR(dev));


    /*---------------------------------------------------------*/
    /* Step 2: Create class                                      */
    /*---------------------------------------------------------*/

    pr_info("[INIT] Calling class_create()\n");

    dev_class = class_create("my_class");

    if (IS_ERR(dev_class)) {

        ret = PTR_ERR(dev_class);

        pr_err("[ERROR] class_create() failed\n");
        pr_err("[ERROR] ret = %d\n", ret);

        goto r_class;
    }

    pr_info("[INIT] Class created successfully\n");
    pr_info("[INIT] /sys/class/my_class should now exist\n");


    /*---------------------------------------------------------*/
    /* Step 3: Create device                                     */
    /*---------------------------------------------------------*/

    pr_info("[INIT] Calling device_create()\n");

    dev_device = device_create(
                    dev_class,
                    NULL,
                    dev,
                    NULL,
                    "my_device"
                 );

    if (IS_ERR(dev_device)) {

        ret = PTR_ERR(dev_device);

        pr_err("[ERROR] device_create() failed\n");
        pr_err("[ERROR] ret = %d\n", ret);

        goto r_device;
    }

    pr_info("[INIT] Device created successfully\n");
    pr_info("[INIT] /dev/my_device should now exist\n");

    pr_info("[INIT] Kernel Module inserted successfully\n");

    return 0;


/*-------------------------------------------------------------*/
/* Cleanup if device_create() fails                            */
/*-------------------------------------------------------------*/

r_device:

    pr_info("[CLEANUP] Destroying class\n");

    class_destroy(dev_class);


/*-------------------------------------------------------------*/
/* Cleanup if class_create() or device_create() fails          */
/*-------------------------------------------------------------*/

r_class:

    pr_info("[CLEANUP] Unregistering device number\n");

    unregister_chrdev_region(dev, 1);

    pr_info("[CLEANUP] Cleanup complete\n");

    return ret;
}


static void __exit hello_exit(void)
{
    pr_info("[EXIT] Module removal started\n");

    pr_info("[EXIT] Destroying device\n");

    device_destroy(dev_class, dev);

    pr_info("[EXIT] Destroying class\n");

    class_destroy(dev_class);

    pr_info("[EXIT] Unregistering device number\n");

    unregister_chrdev_region(dev, 1);

    pr_info("[EXIT] Kernel Module Removed Successfully\n");
}


module_init(hello_init);
module_exit(hello_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sriram S");
MODULE_DESCRIPTION("Device File Creation");
MODULE_VERSION("1.0");
