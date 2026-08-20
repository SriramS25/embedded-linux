/*
 * ============================================================================
 *                  SIMPLE LINUX CHARACTER DRIVER - IOCTL
 * ============================================================================
 *
 * File        : 08_mychar_ioctl.c
 * Author      : Sriram S
 *
 * Description :
 *     Educational character device driver demonstrating ioctl().
 *
 *     Supported commands:
 *
 *          SET_VALUE
 *              User -> Kernel
 *
 *          GET_VALUE
 *              Kernel -> User
 *
 *
 * Userspace device:
 *
 *          /dev/mychar_ioctl
 *
 *
 * Basic flow:
 *
 *      USER SPACE
 *           |
 *           | ioctl(fd, command, argument)
 *           v
 *          VFS
 *           |
 *           v
 *      file_operations
 *           |
 *           | .unlocked_ioctl
 *           v
 *      mychar_ioctl()
 *           |
 *           +---- SET_VALUE
 *           |
 *           +---- GET_VALUE
 *
 * ============================================================================
 */


/* ---------------------------------------------------------------------------
 * Kernel headers
 * ------------------------------------------------------------------------- */

#include <linux/module.h>       /* module_init(), module_exit(),
                                   MODULE_*, THIS_MODULE */

#include <linux/kernel.h>       /* pr_info(), pr_err() */

#include <linux/fs.h>           /* struct file_operations,
                                   struct file */

#include <linux/cdev.h>         /* struct cdev,
                                   cdev_init(), cdev_add(), cdev_del() */

#include <linux/device.h>       /* class_create(), device_create(),
                                   class_destroy(), device_destroy() */

#include <linux/uaccess.h>      /* copy_from_user(),
                                   copy_to_user() */

#include <linux/ioctl.h>        /* _IO(), _IOR(), _IOW(), _IOWR() */


/* ============================================================================
 *                          DEVICE INFORMATION
 * ========================================================================== */

#define DEVICE_NAME "mychar_ioctl"
#define CLASS_NAME  "mychar_ioctl_class"


/* ============================================================================
 *                          IOCTL DEFINITIONS
 * ========================================================================== */


/*
 * MY_MAGIC
 * --------
 *
 * Used to identify our ioctl command group.
 *
 * It helps distinguish our commands from commands belonging
 * to other drivers.
 */
#define MY_MAGIC 'M'


/*
 * SET_VALUE
 * ---------
 *
 * _IOW means:
 *
 *      User -> Kernel
 *
 *
 * Parameters:
 *
 *      MY_MAGIC    -> command group
 *      1           -> command number
 *      int         -> type of data being transferred
 *
 *
 * Userspace will use:
 *
 *      ioctl(fd, SET_VALUE, &value);
 */
#define SET_VALUE _IOW(MY_MAGIC, 1, int)


/*
 * GET_VALUE
 * ---------
 *
 * _IOR means:
 *
 *      Kernel -> User
 *
 *
 * Userspace will use:
 *
 *      ioctl(fd, GET_VALUE, &value);
 */
#define GET_VALUE _IOR(MY_MAGIC, 2, int)


/*
 * IMPORTANT:
 *
 * R/W direction in _IOR/_IOW is from the USERSPACE perspective.
 *
 *
 *      _IOW
 *          USER writes data
 *          USER -> KERNEL
 *
 *      _IOR
 *          USER reads data
 *          KERNEL -> USER
 *
 *      _IOWR
 *          USER <-> KERNEL
 */


/* ============================================================================
 *                          DRIVER OBJECTS
 * ========================================================================== */


/*
 * Device number.
 *
 * Contains:
 *
 *      Major number
 *      Minor number
 */
static dev_t dev;


/*
 * Character device structure.
 */
static struct cdev my_cdev;


/*
 * Device class.
 */
static struct class *my_class;


/*
 * Device object.
 */
static struct device *my_device;


/*
 * Data maintained by our driver.
 *
 * This is what SET_VALUE modifies
 * and GET_VALUE retrieves.
 */
static int my_value;


/* ============================================================================
 *                          IOCTL CALLBACK
 * ========================================================================== */


/*
 * This function is called by the kernel when userspace executes:
 *
 *      ioctl(fd, command, argument);
 *
 *
 * Parameters:
 *
 *      file
 *          Represents the opened device.
 *
 *      cmd
 *          IOCTL command number.
 *
 *      arg
 *          Argument passed from userspace.
 *
 *
 * NOTE:
 *
 * 'arg' is an unsigned long containing a userspace address
 * when the ioctl command requires data transfer.
 */
static long mychar_ioctl(struct file *file,
                         unsigned int cmd,
                         unsigned long arg)
{
        int value;


        pr_info("[mychar_ioctl]: ioctl() called\n");

        pr_info("[mychar_ioctl]: command = 0x%x\n", cmd);


        /*
         * Check which command userspace requested.
         */
        switch (cmd) {


        /* =================================================================
         *                         SET_VALUE
         * ================================================================= */

        case SET_VALUE:

                pr_info("[mychar_ioctl]: SET_VALUE received\n");


                /*
                 * 'arg' contains the userspace address
                 * of the integer.
                 *
                 * We CANNOT directly do:
                 *
                 *      my_value = *(int *)arg;
                 *
                 * because arg points to userspace memory.
                 *
                 * Instead we safely copy the data using
                 * copy_from_user().
                 */
                if (copy_from_user(&value,
                                   (int __user *)arg,
                                   sizeof(value))) {

                        pr_err("[mychar_ioctl]: "
                               "copy_from_user() failed\n");

                        return -EFAULT;
                }


                /*
                 * Now 'value' is safely inside kernel memory.
                 *
                 * Store it in our driver's variable.
                 */
                my_value = value;


                pr_info("[mychar_ioctl]: "
                        "my_value = %d\n",
                        my_value);

                break;


        /* =================================================================
         *                         GET_VALUE
         * ================================================================= */

        case GET_VALUE:

                pr_info("[mychar_ioctl]: GET_VALUE received\n");


                /*
                 * Copy the driver's value into a local
                 * kernel variable first.
                 */
                value = my_value;


                /*
                 * Copy kernel data to userspace.
                 *
                 * Destination:
                 *
                 *      (int __user *)arg
                 *
                 * Source:
                 *
                 *      &value
                 */
                if (copy_to_user((int __user *)arg,
                                 &value,
                                 sizeof(value))) {

                        pr_err("[mychar_ioctl]: "
                               "copy_to_user() failed\n");

                        return -EFAULT;
                }


                pr_info("[mychar_ioctl]: "
                        "Returned value = %d\n",
                        value);

                break;


        /* =================================================================
         *                         UNKNOWN COMMAND
         * ================================================================= */

        default:

                pr_err("[mychar_ioctl]: "
                       "Unknown ioctl command: 0x%x\n",
                       cmd);

                return -EINVAL;
        }


        /*
         * Returning 0 means:
         *
         *      ioctl operation succeeded.
         */
        return 0;
}


/* ============================================================================
 *                          FILE OPERATIONS
 * ========================================================================== */


/*
 * This connects the ioctl system call to our callback.
 *
 *
 * Userspace:
 *
 *      ioctl()
 *         |
 *         v
 *      VFS
 *         |
 *         v
 *      .unlocked_ioctl
 *         |
 *         v
 *      mychar_ioctl()
 */
static const struct file_operations my_fops = {

        .owner = THIS_MODULE,

        .unlocked_ioctl = mychar_ioctl,
};


/* ============================================================================
 *                         MODULE INITIALIZATION
 * ========================================================================== */


/*
 * Called when:
 *
 *      sudo insmod 08_mychar_ioctl.ko
 */
static int __init mychar_driver_init(void)
{
        int ret;


        pr_info("[mychar_ioctl]: "
                "Driver initialization started\n");


        /* -----------------------------------------------------------------
         * STEP 1
         *
         * Allocate major/minor number.
         * ----------------------------------------------------------------- */

        ret = alloc_chrdev_region(&dev,
                                  0,
                                  1,
                                  DEVICE_NAME);

        if (ret < 0) {

                pr_err("[mychar_ioctl]: "
                       "alloc_chrdev_region() failed\n");

                return ret;
        }


        pr_info("[mychar_ioctl]: "
                "MAJOR = %d MINOR = %d\n",
                MAJOR(dev),
                MINOR(dev));


        /* -----------------------------------------------------------------
         * STEP 2
         *
         * Initialize cdev and connect it to file operations.
         * ----------------------------------------------------------------- */

        cdev_init(&my_cdev,
                  &my_fops);


        /* -----------------------------------------------------------------
         * STEP 3
         *
         * Register cdev with kernel.
         * ----------------------------------------------------------------- */

        ret = cdev_add(&my_cdev,
                       dev,
                       1);

        if (ret < 0) {

                pr_err("[mychar_ioctl]: "
                       "cdev_add() failed\n");

                goto err_cdev;
        }


        /* -----------------------------------------------------------------
         * STEP 4
         *
         * Create device class.
         * ----------------------------------------------------------------- */

        my_class = class_create(CLASS_NAME);

        if (IS_ERR(my_class)) {

                ret = PTR_ERR(my_class);

                pr_err("[mychar_ioctl]: "
                       "class_create() failed\n");

                goto err_class;
        }


        /* -----------------------------------------------------------------
         * STEP 5
         *
         * Create /dev/mychar_ioctl
         * ----------------------------------------------------------------- */

        my_device = device_create(my_class,
                                  NULL,
                                  dev,
                                  NULL,
                                  DEVICE_NAME);

        if (IS_ERR(my_device)) {

                ret = PTR_ERR(my_device);

                pr_err("[mychar_ioctl]: "
                       "device_create() failed\n");

                goto err_device;
        }


        /*
         * Initial driver value.
         */
        my_value = 0;


        pr_info("[mychar_ioctl]: "
                "Driver initialized successfully\n");

        pr_info("[mychar_ioctl]: "
                "Device: /dev/%s\n",
                DEVICE_NAME);


        return 0;


/* ============================================================================
 *                          ERROR CLEANUP
 * ========================================================================== */

err_device:

        class_destroy(my_class);

err_class:

        cdev_del(&my_cdev);

err_cdev:

        unregister_chrdev_region(dev, 1);

        return ret;
}


/* ============================================================================
 *                          MODULE CLEANUP
 * ========================================================================== */


/*
 * Called when:
 *
 *      sudo rmmod 08_mychar_ioctl
 */
static void __exit mychar_driver_exit(void)
{
        device_destroy(my_class, dev);

        class_destroy(my_class);

        cdev_del(&my_cdev);

        unregister_chrdev_region(dev, 1);


        pr_info("[mychar_ioctl]: "
                "Driver cleanup completed\n");
}


/* ============================================================================
 *                          MODULE REGISTRATION
 * ========================================================================== */

module_init(mychar_driver_init);

module_exit(mychar_driver_exit);


/* ============================================================================
 *                          MODULE INFORMATION
 * ========================================================================== */

MODULE_LICENSE("GPL");

MODULE_AUTHOR("Sriram S");

MODULE_DESCRIPTION(
        "Educational Linux character driver demonstrating IOCTL"
);

MODULE_VERSION("1.0");



/*[294108.353395] [mychar_ioctl]: Driver initialization started
[294108.353454] [mychar_ioctl]: MAJOR = 234 MINOR = 0
[294108.354847] [mychar_ioctl]: Driver initialized successfully
[294108.354906] [mychar_ioctl]: Device: /dev/mychar_ioctl
[294124.586254] [mychar_ioctl]: ioctl() called
[294124.586307] [mychar_ioctl]: command = 0x40044d01
[294124.586323] [mychar_ioctl]: SET_VALUE received
[294124.586361] [mychar_ioctl]: my_value = 100
[294124.586443] [mychar_ioctl]: ioctl() called
[294124.586467] [mychar_ioctl]: command = 0x80044d02
[294124.586486] [mychar_ioctl]: GET_VALUE received
[294124.586503] [mychar_ioctl]: Returned value = 100
[294407.965788] [mychar_ioctl]: Driver cleanup completed
*/
