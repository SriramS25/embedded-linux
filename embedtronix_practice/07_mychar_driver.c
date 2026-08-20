/*
 * ============================================================================
 *                         SIMPLE LINUX CHARACTER DRIVER
 * ============================================================================
 *
 * File        : 07_mychar_driver.c
 * Author      : Sriram S
 *
 * Description :
 *     Educational Linux character device driver demonstrating:
 *
 *       1. Dynamic major/minor allocation
 *       2. Character device registration
 *       3. Linux device class creation
 *       4. /dev device creation
 *       5. File operation callbacks
 *       6. Kernel memory allocation
 *       7. User -> Kernel data transfer
 *       8. Kernel -> User data transfer
 *       9. Proper error handling and cleanup
 *
 *
 * User-space interface:
 *
 *       /dev/mychar_dev
 *
 *
 * Data flow:
 *
 *       USER SPACE
 *           |
 *           | write()
 *           v
 *       mychar_write()
 *           |
 *           | copy_from_user()
 *           v
 *       kernel_buffer
 *           |
 *           | copy_to_user()
 *           v
 *       mychar_read()
 *           |
 *           v
 *       USER SPACE
 *
 * ============================================================================
 */


#include <linux/module.h>       /* module_init(), module_exit(),
                                  MODULE_*, THIS_MODULE */

#include <linux/kernel.h>       /* pr_info(), pr_err() */

#include <linux/init.h>         /* __init, __exit */

#include <linux/kdev_t.h>       /* dev_t, MAJOR(), MINOR() */

#include <linux/fs.h>           /* struct file_operations,
                                  struct file, struct inode */

#include <linux/cdev.h>         /* struct cdev, cdev_init(),
                                  cdev_add(), cdev_del() */

#include <linux/device.h>       /* class_create(), device_create(),
                                  class_destroy(), device_destroy() */

#include <linux/slab.h>         /* kmalloc(), kfree() */

#include <linux/uaccess.h>      /* copy_to_user(), copy_from_user() */

#include <linux/err.h>          /* IS_ERR(), PTR_ERR() */


/* ============================================================================
 *                              DRIVER DEFINITIONS
 * ========================================================================== */

#define DEVICE_NAME  "mychar_dev"
#define CLASS_NAME   "mychar_class"
#define BUFFER_SIZE  1024


/* ============================================================================
 *                         DRIVER GLOBAL OBJECTS
 * ========================================================================== */


/*
 * dev_t
 * -----
 *
 * Stores the device number:
 *
 *              MAJOR : MINOR
 *
 * Example:
 *
 *              235 : 0
 */
static dev_t dev;


/*
 * struct cdev
 * -----------
 *
 * Represents our character device inside the kernel.
 */
static struct cdev my_cdev;


/*
 * Device class.
 *
 * Represents our driver class in the Linux device model.
 */
static struct class *mychar_class;


/*
 * Device instance.
 *
 * Represents the actual device created using device_create().
 */
static struct device *mychar_device;


/*
 * Kernel buffer.
 *
 * This buffer stores data written by userspace.
 *
 * Size:
 *
 *              1024 bytes
 */
static char *kernel_buffer;


/*
 * Number of valid bytes currently stored in kernel_buffer.
 *
 * Example:
 *
 * If userspace writes:
 *
 *              "hello\n"
 *
 * then:
 *
 *              data_size = 6
 */
static size_t data_size;


/* ============================================================================
 *                         FUNCTION PROTOTYPES
 * ========================================================================== */

static int mychar_open(struct inode *inode,
                       struct file *file);

static int mychar_release(struct inode *inode,
                          struct file *file);

static ssize_t mychar_read(struct file *file,
                           char __user *buf,
                           size_t len,
                           loff_t *offset);

static ssize_t mychar_write(struct file *file,
                            const char __user *buf,
                            size_t len,
                            loff_t *offset);


/* ============================================================================
 *                         FILE OPERATIONS TABLE
 * ========================================================================== */


/*
 * This structure connects VFS operations with our driver callbacks.
 *
 *
 *       userspace operation
 *               |
 *               v
 *              VFS
 *               |
 *               v
 *       struct file_operations
 *               |
 *       +-------+--------+---------+
 *       |       |        |         |
 *       v       v        v         v
 *      open    read     write    release
 *       |       |        |         |
 *       v       v        v         v
 *   mychar_  mychar_  mychar_  mychar_
 *    open     read     write    release
 */
static const struct file_operations my_fops = {

        .owner   = THIS_MODULE,

        .open    = mychar_open,

        .read    = mychar_read,

        .write   = mychar_write,

        .release = mychar_release,
};


/* ============================================================================
 *                              OPEN
 * ========================================================================== */


/*
 * Called when userspace opens:
 *
 *              /dev/mychar_dev
 *
 * Example:
 *
 *              fd = open("/dev/mychar_dev", O_RDWR);
 */
static int mychar_open(struct inode *inode,
                       struct file *file)
{
        pr_info("[mychar_driver]: open() called\n");

        pr_info("[mychar_driver]: MAJOR = %d MINOR = %d\n",
                MAJOR(inode->i_rdev),
                MINOR(inode->i_rdev));

        return 0;
}


/* ============================================================================
 *                              WRITE
 * ========================================================================== */


/*
 * Called when userspace performs:
 *
 *              write(fd, buffer, length);
 *
 *
 * Data flow:
 *
 *       USER SPACE
 *
 *       user buffer
 *            |
 *            | copy_from_user()
 *            v
 *
 *       KERNEL SPACE
 *
 *       kernel_buffer
 */
static ssize_t mychar_write(struct file *file,
                            const char __user *buf,
                            size_t len,
                            loff_t *offset)
{
        pr_info("[mychar_driver]: write() called\n");

        pr_info("[mychar_driver]: Requested length = %zu\n",
                len);


        /*
         * Prevent buffer overflow.
         *
         * kernel_buffer can store only BUFFER_SIZE bytes.
         */
        if (len > BUFFER_SIZE) {

                pr_err("[mychar_driver]: "
                       "Input too large: %zu bytes\n",
                       len);

                return -EINVAL;
        }


        /*
         * Copy data from userspace into kernel space.
         *
         * Destination:
         *
         *              kernel_buffer
         *
         * Source:
         *
         *              buf
         *
         * Number of bytes:
         *
         *              len
         */
        if (copy_from_user(kernel_buffer, buf, len)) {

                pr_err("[mychar_driver]: "
                       "copy_from_user() failed\n");

                return -EFAULT;
        }


        /*
         * Remember how many bytes are valid.
         */
        data_size = len;


        /*
         * Reset file position.
         *
         * The next read starts from the beginning.
         */
        *offset = 0;


        pr_info("[mychar_driver]: "
                "%zu bytes written to kernel buffer\n",
                len);


        /*
         * Return number of bytes successfully consumed.
         */
        return len;
}


/* ============================================================================
 *                              READ
 * ========================================================================== */


/*
 * Called when userspace performs:
 *
 *              read(fd, buffer, length);
 *
 *
 * Data flow:
 *
 *       KERNEL SPACE
 *
 *       kernel_buffer
 *            |
 *            | copy_to_user()
 *            v
 *
 *       USER SPACE
 *
 *       user buffer
 */
static ssize_t mychar_read(struct file *file,
                           char __user *buf,
                           size_t len,
                           loff_t *offset)
{
        size_t bytes_to_copy;


        pr_info("[mychar_driver]: read() called\n");

        pr_info("[mychar_driver]: "
                "Requested length = %zu\n",
                len);

        pr_info("[mychar_driver]: "
                "Current offset = %lld\n",
                *offset);


        /*
         * If the current file offset is already at or beyond
         * the valid data, return 0.
         *
         * Returning 0 means EOF.
         */
        if (*offset >= data_size) {

                pr_info("[mychar_driver]: "
                        "End of data reached\n");

                return 0;
        }


        /*
         * Calculate how much data remains.
         */
        bytes_to_copy = data_size - *offset;


        /*
         * User requested less than the remaining data.
         *
         * Therefore copy only 'len' bytes.
         */
        if (bytes_to_copy > len)
                bytes_to_copy = len;


        /*
         * Copy data from kernel space to userspace.
         */
        if (copy_to_user(buf,
                         kernel_buffer + *offset,
                         bytes_to_copy)) {

                pr_err("[mychar_driver]: "
                       "copy_to_user() failed\n");

                return -EFAULT;
        }


        /*
         * Update file position.
         *
         * Example:
         *
         * Initially:
         *
         *              offset = 0
         *
         * Read 5 bytes:
         *
         *              offset = 5
         */
        *offset += bytes_to_copy;


        pr_info("[mychar_driver]: "
                "%zu bytes read from kernel buffer\n",
                bytes_to_copy);


        /*
         * Return number of bytes actually copied.
         */
        return bytes_to_copy;
}


/* ============================================================================
 *                              RELEASE
 * ========================================================================== */


/*
 * Called when userspace closes the device:
 *
 *              close(fd);
 */
static int mychar_release(struct inode *inode,
                          struct file *file)
{
        pr_info("[mychar_driver]: release() called\n");

        return 0;
}


/* ============================================================================
 *                         MODULE INITIALIZATION
 * ========================================================================== */


/*
 * Called when:
 *
 *              sudo insmod 07_mychar_driver.ko
 *
 *
 * Initialization sequence:
 *
 *      1. alloc_chrdev_region()
 *      2. cdev_init()
 *      3. cdev_add()
 *      4. class_create()
 *      5. device_create()
 *      6. kmalloc()
 */
static int __init mychar_driver_init(void)
{
        int ret;


        pr_info("[mychar_driver]: "
                "Driver Initialization Started\n");


        /* -----------------------------------------------------------------
         * STEP 1
         *
         * Allocate major/minor number dynamically.
         * ----------------------------------------------------------------- */

        ret = alloc_chrdev_region(&dev,
                                  0,
                                  1,
                                  DEVICE_NAME);

        if (ret < 0) {

                pr_err("[mychar_driver]: "
                       "alloc_chrdev_region() failed: %d\n",
                       ret);

                return ret;
        }


        pr_info("[mychar_driver]: "
                "MAJOR = %d MINOR = %d\n",
                MAJOR(dev),
                MINOR(dev));


        /* -----------------------------------------------------------------
         * STEP 2
         *
         * Initialize cdev and connect it to file_operations.
         * ----------------------------------------------------------------- */

        cdev_init(&my_cdev, &my_fops);

        pr_info("[mychar_driver]: "
                "cdev initialized\n");


        /* -----------------------------------------------------------------
         * STEP 3
         *
         * Register cdev with the kernel.
         * ----------------------------------------------------------------- */

        ret = cdev_add(&my_cdev,
                       dev,
                       1);

        if (ret < 0) {

                pr_err("[mychar_driver]: "
                       "cdev_add() failed: %d\n",
                       ret);

                goto err_cdev;
        }


        pr_info("[mychar_driver]: "
                "cdev added successfully\n");


        /* -----------------------------------------------------------------
         * STEP 4
         *
         * Create device class.
         * ----------------------------------------------------------------- */

        mychar_class = class_create(CLASS_NAME);

        if (IS_ERR(mychar_class)) {

                ret = PTR_ERR(mychar_class);

                pr_err("[mychar_driver]: "
                       "class_create() failed: %d\n",
                       ret);

                goto err_class;
        }


        pr_info("[mychar_driver]: "
                "class created successfully\n");


        /* -----------------------------------------------------------------
         * STEP 5
         *
         * Create actual device.
         *
         * This results in:
         *
         *              /dev/mychar_dev
         * ----------------------------------------------------------------- */

        mychar_device = device_create(mychar_class,
                                      NULL,
                                      dev,
                                      NULL,
                                      DEVICE_NAME);

        if (IS_ERR(mychar_device)) {

                ret = PTR_ERR(mychar_device);

                pr_err("[mychar_driver]: "
                       "device_create() failed: %d\n",
                       ret);

                goto err_device;
        }


        pr_info("[mychar_driver]: "
                "device created successfully\n");


        /* -----------------------------------------------------------------
         * STEP 6
         *
         * Allocate kernel buffer.
         * ----------------------------------------------------------------- */

        kernel_buffer = kmalloc(BUFFER_SIZE,
                                GFP_KERNEL);

        if (!kernel_buffer) {

                pr_err("[mychar_driver]: "
                       "kmalloc() failed\n");

                ret = -ENOMEM;

                goto err_buffer;
        }


        /*
         * Initially there is no valid data.
         */
        data_size = 0;


        /*
         * Optional:
         *
         * Clear the allocated memory.
         */
        memset(kernel_buffer, 0, BUFFER_SIZE);


        pr_info("[mychar_driver]: "
                "Kernel buffer allocated: %d bytes\n",
                BUFFER_SIZE);


        pr_info("[mychar_driver]: "
                "/dev/%s is ready\n",
                DEVICE_NAME);


        pr_info("[mychar_driver]: "
                "Driver Initialization Completed\n");


        return 0;


/* ============================================================================
 *                         ERROR CLEANUP
 * ========================================================================== */


/*
 * kmalloc() failed.
 *
 * Device exists.
 * Class exists.
 * cdev exists.
 * Device number exists.
 */
err_buffer:

        device_destroy(mychar_class, dev);


err_device:

        class_destroy(mychar_class);


err_class:

        cdev_del(&my_cdev);


err_cdev:

        unregister_chrdev_region(dev, 1);

        return ret;
}


/* ============================================================================
 *                           MODULE CLEANUP
 * ========================================================================== */


/*
 * Called when:
 *
 *              sudo rmmod 07_mychar_driver
 *
 *
 * Resources are released in reverse order:
 *
 *      kmalloc()
 *          |
 *          v
 *      kfree()
 *
 *      device_create()
 *          |
 *          v
 *      device_destroy()
 *
 *      class_create()
 *          |
 *          v
 *      class_destroy()
 *
 *      cdev_add()
 *          |
 *          v
 *      cdev_del()
 *
 *      alloc_chrdev_region()
 *          |
 *          v
 *      unregister_chrdev_region()
 */
static void __exit mychar_driver_exit(void)
{
        /*
         * Release kernel memory.
         */
        kfree(kernel_buffer);

        pr_info("[mychar_driver]: "
                "Kernel buffer freed\n");


        /*
         * Remove device.
         */
        device_destroy(mychar_class, dev);

        pr_info("[mychar_driver]: "
                "Device destroyed\n");


        /*
         * Destroy class.
         */
        class_destroy(mychar_class);

        pr_info("[mychar_driver]: "
                "Class destroyed\n");


        /*
         * Remove cdev from kernel.
         */
        cdev_del(&my_cdev);

        pr_info("[mychar_driver]: "
                "cdev deleted\n");


        /*
         * Release major/minor number.
         */
        unregister_chrdev_region(dev, 1);

        pr_info("[mychar_driver]: "
                "Device number released\n");


        pr_info("[mychar_driver]: "
                "Driver Cleanup Completed\n");
}


/* ============================================================================
 *                         MODULE REGISTRATION
 * ========================================================================== */

module_init(mychar_driver_init);

module_exit(mychar_driver_exit);


/* ============================================================================
 *                         MODULE INFORMATION
 * ========================================================================== */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sriram S");
MODULE_DESCRIPTION(
        "Educational Linux character device driver demonstrating "
        "VFS file operations and user-kernel data transfer"
);
MODULE_VERSION("1.1");
