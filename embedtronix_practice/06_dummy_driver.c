/*
 * ============================================================================
 *                  SIMPLE DUMMY CHARACTER DEVICE DRIVER
 * ============================================================================
 *
 * This driver does NOT communicate with any real hardware.
 *
 * It demonstrates the basic Linux character-driver architecture:
 *
 *      Module
 *        |
 *        v
 *  alloc_chrdev_region()
 *        |
 *        v
 *     major/minor
 *        |
 *        v
 *    cdev_init()
 *        |
 *        v
 *     cdev_add()
 *        |
 *        v
 *    class_create()
 *        |
 *        v
 *    device_create()
 *        |
 *        v
 *   /dev/etx_device
 *        |
 *        v
 *      USER
 *
 *
 * Userspace operations:
 *
 *      open("/dev/etx_device")
 *              |
 *              v
 *          etx_open()
 *
 *      read(fd, ...)
 *              |
 *              v
 *          etx_read()
 *
 *      write(fd, ...)
 *              |
 *              v
 *          etx_write()
 *
 *      close(fd)
 *              |
 *              v
 *          etx_release()
 *
 * ============================================================================
 */

#include <linux/kernel.h>      /* pr_info(), pr_err() */
#include <linux/init.h>        /* __init, __exit */
#include <linux/module.h>      /* module_init(), module_exit(), THIS_MODULE */
#include <linux/kdev_t.h>      /* dev_t, MAJOR(), MINOR() */
#include <linux/fs.h>          /* struct file, struct inode,
                                  struct file_operations */
#include <linux/cdev.h>        /* struct cdev, cdev_init(),
                                  cdev_add(), cdev_del() */
#include <linux/device.h>      /* class_create(), device_create(),
                                  class_destroy(), device_destroy() */
#include <linux/err.h>         /* IS_ERR(), PTR_ERR() */


/* ============================================================================
 *                            DEVICE INFORMATION
 * ========================================================================== */

#define DEVICE_NAME "etx_device"
#define CLASS_NAME  "etx_class"


/*
 * dev_t
 * -----
 *
 * Stores the device number.
 *
 * Device number consists of:
 *
 *      MAJOR : MINOR
 *
 * Example:
 *
 *      509 : 0
 *
 * Major -> identifies the driver
 * Minor -> identifies a particular device handled by that driver
 */
static dev_t dev;


/*
 * struct cdev
 * -----------
 *
 * Represents our character device inside the kernel.
 *
 * Later we connect this cdev with our file_operations structure.
 */
static struct cdev etx_cdev;


/*
 * struct class
 * ------------
 *
 * Represents our device class in the Linux device model.
 *
 * It is used together with device_create() to expose the device
 * through the normal Linux device-management mechanism.
 */
static struct class *etx_class;


/* ============================================================================
 *                         FUNCTION PROTOTYPES
 * ========================================================================== */

/*
 * These are OUR functions.
 *
 * They are not called directly by userspace.
 *
 * They are registered with the kernel through struct file_operations.
 *
 * The VFS calls them when userspace performs open/read/write/close.
 */

static int etx_open(struct inode *inode, struct file *file);

static int etx_release(struct inode *inode, struct file *file);

static ssize_t etx_read(struct file *file,
                        char __user *buf,
                        size_t len,
                        loff_t *offset);

static ssize_t etx_write(struct file *file,
                         const char __user *buf,
                         size_t len,
                         loff_t *offset);


/* ============================================================================
 *                         FILE OPERATIONS TABLE
 * ========================================================================== */

/*
 * struct file_operations
 * ----------------------
 *
 * This is a TABLE OF FUNCTION POINTERS.
 *
 * We are telling the VFS:
 *
 *      If userspace opens our device:
 *              call etx_open()
 *
 *      If userspace reads:
 *              call etx_read()
 *
 *      If userspace writes:
 *              call etx_write()
 *
 *      If userspace closes:
 *              call etx_release()
 *
 *
 * This is the callback mechanism used by the VFS.
 */
static const struct file_operations etx_fops = {

        /*
         * Tell the kernel that these operations belong to this module.
         */
        .owner = THIS_MODULE,

        /*
         * Callback for open()
         */
        .open = etx_open,

        /*
         * Callback for read()
         */
        .read = etx_read,

        /*
         * Callback for write()
         */
        .write = etx_write,

        /*
         * Callback for close()/release
         */
        .release = etx_release,
};


/* ============================================================================
 *                              OPEN
 * ========================================================================== */

/*
 * This function is called when userspace does:
 *
 *      open("/dev/etx_device", ...);
 *
 * Flow:
 *
 *      userspace
 *          |
 *          v
 *      open()
 *          |
 *          v
 *         VFS
 *          |
 *          v
 *      etx_fops.open
 *          |
 *          v
 *      etx_open()
 */
static int etx_open(struct inode *inode, struct file *file)
{
        pr_info("ETX: etx_open() called\n");

        /*
         * inode->i_rdev contains the device number associated
         * with this device.
         */
        pr_info("ETX: Major = %d, Minor = %d\n",
                MAJOR(inode->i_rdev),
                MINOR(inode->i_rdev));

        /*
         * Returning 0 means:
         *
         *      open() succeeded.
         */
        return 0;
}


/* ============================================================================
 *                              READ
 * ========================================================================== */

/*
 * This function is called when userspace does:
 *
 *      read(fd, buffer, size);
 *
 * For this DUMMY driver, we are not actually returning any data.
 *
 * Returning 0 means:
 *
 *      EOF (End Of File)
 *
 * Therefore:
 *
 *      cat /dev/etx_device
 *
 * will call this function and immediately receive EOF.
 */
static ssize_t etx_read(struct file *file,
                        char __user *buf,
                        size_t len,
                        loff_t *offset)
{
        pr_info("ETX: etx_read() called\n");

        pr_info("ETX: Requested bytes = %zu\n", len);

        pr_info("ETX: Current file offset = %lld\n",
                *offset);

        /*
         * Returning 0 means:
         *
         *      There is no data available.
         */
        return 0;
}


/* ============================================================================
 *                              WRITE
 * ========================================================================== */

/*
 * This function is called when userspace does:
 *
 *      write(fd, buffer, size);
 *
 * For this DUMMY driver, we are NOT actually storing the data.
 *
 * We simply print a debug message and report that all bytes
 * were successfully "consumed".
 *
 * IMPORTANT:
 *
 * A real driver must normally use copy_from_user() to safely
 * copy data from userspace into kernel memory.
 */
static ssize_t etx_write(struct file *file,
                         const char __user *buf,
                         size_t len,
                         loff_t *offset)
{
        pr_info("ETX: etx_write() called\n");

        pr_info("ETX: Requested bytes = %zu\n", len);

        /*
         * Returning len tells the VFS:
         *
         *      "I successfully consumed all len bytes."
         *
         * Since this is only a dummy driver, we don't actually
         * store the data anywhere.
         */
        return len;
}


/* ============================================================================
 *                             RELEASE
 * ========================================================================== */

/*
 * This function is called when userspace does:
 *
 *      close(fd);
 *
 * Example:
 *
 *      int fd;
 *
 *      fd = open("/dev/etx_device", O_RDWR);
 *
 *      close(fd);
 *
 * Flow:
 *
 *      close()
 *        |
 *        v
 *       VFS
 *        |
 *        v
 *   etx_fops.release
 *        |
 *        v
 *   etx_release()
 */
static int etx_release(struct inode *inode,
                       struct file *file)
{
        pr_info("ETX: etx_release() called\n");

        return 0;
}


/* ============================================================================
 *                         MODULE INITIALIZATION
 * ========================================================================== */

/*
 * This function runs when the module is inserted:
 *
 *      sudo insmod etx_driver.ko
 *
 *
 * We perform the following operations:
 *
 *      1. Allocate major/minor number
 *      2. Initialize cdev
 *      3. Add cdev to kernel
 *      4. Create device class
 *      5. Create device
 *
 * After this, userspace should have:
 *
 *      /dev/etx_device
 */
static int __init etx_driver_init(void)
{
        int ret;

        pr_info("ETX: Driver initialization started\n");


        /* -----------------------------------------------------------------
         * STEP 1:
         *
         * Dynamically allocate a major/minor number.
         *
         * We request:
         *
         *      first minor = 0
         *      number of devices = 1
         *
         * The kernel chooses the major number.
         * ----------------------------------------------------------------- */

        ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);

        if (ret < 0) {

                pr_err("ETX: alloc_chrdev_region() failed\n");

                return ret;
        }

        pr_info("ETX: Device number allocated successfully\n");

        pr_info("ETX: Major = %d, Minor = %d\n",
                MAJOR(dev),
                MINOR(dev));


        /* -----------------------------------------------------------------
         * STEP 2:
         *
         * Initialize our cdev structure.
         *
         * Connect:
         *
         *      etx_cdev
         *          |
         *          v
         *      etx_fops
         *
         * ----------------------------------------------------------------- */

        cdev_init(&etx_cdev, &etx_fops);

        /*
         * Explicitly associate the cdev with this module.
         */
        etx_cdev.owner = THIS_MODULE;

        pr_info("ETX: cdev initialized\n");


        /* -----------------------------------------------------------------
         * STEP 3:
         *
         * Add the character device to the kernel.
         *
         * After this, the kernel knows:
         *
         *      Device number -> etx_cdev -> etx_fops
         *
         * ----------------------------------------------------------------- */

        ret = cdev_add(&etx_cdev, dev, 1);

        if (ret < 0) {

                pr_err("ETX: cdev_add() failed\n");

                goto err_cdev;
        }

        pr_info("ETX: cdev added successfully\n");


        /* -----------------------------------------------------------------
         * STEP 4:
         *
         * Create a device class.
         *
         * Conceptually this gives us:
         *
         *      /sys/class/etx_class/
         *
         * ----------------------------------------------------------------- */

        etx_class = class_create(CLASS_NAME);

        if (IS_ERR(etx_class)) {

                ret = PTR_ERR(etx_class);

                pr_err("ETX: class_create() failed: %d\n",
                       ret);

                goto err_class;
        }

        pr_info("ETX: Device class created\n");


        /* -----------------------------------------------------------------
         * STEP 5:
         *
         * Create the actual device instance.
         *
         * This associates:
         *
         *      class
         *      +
         *      device number
         *      +
         *      device name
         *
         * The normal device-management mechanism can then create:
         *
         *      /dev/etx_device
         *
         * ----------------------------------------------------------------- */

        if (IS_ERR(device_create(etx_class,
                                 NULL,
                                 dev,
                                 NULL,
                                 DEVICE_NAME))) {

                ret = -ENOMEM;

                pr_err("ETX: device_create() failed\n");

                goto err_device;
        }

        pr_info("ETX: Device created successfully\n");

        pr_info("ETX: /dev/%s is ready\n",
                DEVICE_NAME);

        pr_info("ETX: Driver initialization completed\n");

        return 0;


/* ============================================================================
 *                         ERROR CLEANUP
 * ========================================================================== */

err_device:

        /*
         * Undo class_create()
         */
        class_destroy(etx_class);


err_class:

        /*
         * Undo cdev_add()
         */
        cdev_del(&etx_cdev);


err_cdev:

        /*
         * Undo alloc_chrdev_region()
         */
        unregister_chrdev_region(dev, 1);

        return ret;
}


/* ============================================================================
 *                          MODULE CLEANUP
 * ========================================================================== */

/*
 * This function runs when:
 *
 *      sudo rmmod etx_driver
 *
 * We must release resources in the reverse order in which
 * they were created.
 *
 * Initialization:
 *
 *      alloc_chrdev_region()
 *              ↓
 *          cdev_add()
 *              ↓
 *        class_create()
 *              ↓
 *        device_create()
 *
 *
 * Cleanup:
 *
 *        device_destroy()
 *              ↓
 *        class_destroy()
 *              ↓
 *          cdev_del()
 *              ↓
 *   unregister_chrdev_region()
 */
static void __exit etx_driver_exit(void)
{
        pr_info("ETX: Driver cleanup started\n");


        /*
         * Undo device_create()
         */
        device_destroy(etx_class, dev);

        pr_info("ETX: Device destroyed\n");


        /*
         * Undo class_create()
         */
        class_destroy(etx_class);

        pr_info("ETX: Class destroyed\n");


        /*
         * Undo cdev_add()
         */
        cdev_del(&etx_cdev);

        pr_info("ETX: cdev deleted\n");


        /*
         * Release the major/minor number.
         */
        unregister_chrdev_region(dev, 1);

        pr_info("ETX: Device number released\n");

        pr_info("ETX: Driver removed successfully\n");
}


/* ============================================================================
 *                         MODULE REGISTRATION
 * ========================================================================== */

/*
 * Tell the kernel:
 *
 *      When module is inserted:
 *              call etx_driver_init()
 */
module_init(etx_driver_init);


/*
 * Tell the kernel:
 *
 *      When module is removed:
 *              call etx_driver_exit()
 */
module_exit(etx_driver_exit);


/* ============================================================================
 *                          MODULE INFORMATION
 * ========================================================================== */

MODULE_LICENSE("GPL");

MODULE_AUTHOR("Sriram S");

MODULE_DESCRIPTION("Simple dummy Linux character device driver");

MODULE_VERSION("1.0");
