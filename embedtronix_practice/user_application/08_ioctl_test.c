/*
 * ============================================================================
 *                     IOCTL USERSPACE TEST APPLICATION
 * ============================================================================
 *
 * File        : 08_ioctl_test.c
 * Author      : Sriram S
 *
 * Description :
 *     Userspace application used to communicate with the
 *     character driver through ioctl().
 *
 *
 * Supported operations:
 *
 *      1. SET_VALUE
 *         Send an integer from userspace to kernel.
 *
 *      2. GET_VALUE
 *         Retrieve an integer from kernel to userspace.
 *
 * ============================================================================
 */


/* ---------------------------------------------------------------------------
 * Standard headers
 * ------------------------------------------------------------------------- */

#include <stdio.h>              /* printf(), perror() */

#include <stdlib.h>             /* EXIT_SUCCESS, EXIT_FAILURE */

#include <fcntl.h>              /* open(), O_RDWR */

#include <unistd.h>             /* close() */

#include <sys/ioctl.h>          /* ioctl(), _IOW(), _IOR() */


/* ============================================================================
 *                          DEVICE INFORMATION
 * ========================================================================== */

#define DEVICE_NAME "/dev/mychar_ioctl"


/* ============================================================================
 *                          IOCTL DEFINITIONS
 * ========================================================================== */


/*
 * IMPORTANT:
 *
 * These definitions MUST match the definitions used
 * in the kernel driver.
 */
#define MY_MAGIC 'M'


/*
 * SET_VALUE:
 *
 *      USER -> KERNEL
 *
 * The userspace program sends an int to the driver.
 */
#define SET_VALUE _IOW(MY_MAGIC, 1, int)


/*
 * GET_VALUE:
 *
 *      KERNEL -> USER
 *
 * The driver sends an int back to userspace.
 */
#define GET_VALUE _IOR(MY_MAGIC, 2, int)


/* ============================================================================
 *                              MAIN
 * ========================================================================== */

int main(void)
{
        int fd;

        int value;


        /* -----------------------------------------------------------------
         * STEP 1
         *
         * Open the device.
         * ----------------------------------------------------------------- */

        fd = open(DEVICE_NAME,
                  O_RDWR);

        if (fd < 0) {

                perror("open");

                return EXIT_FAILURE;
        }


        printf("Device opened successfully\n");


        /* =================================================================
         *                         SET_VALUE
         * ================================================================= */


        /*
         * Value we want to send to the driver.
         */
        value = 100;


        printf("\n");
        printf("Sending value to driver: %d\n",
               value);


        /*
         * ioctl() call:
         *
         *
         *      fd
         *          Device file descriptor.
         *
         *      SET_VALUE
         *          Command we want the driver to execute.
         *
         *      &value
         *          Address of the userspace variable.
         *
         *
         * Flow:
         *
         *      USER SPACE
         *
         *      value = 100
         *          |
         *          | &value
         *          v
         *
         *      ioctl()
         *          |
         *          v
         *
         *      KERNEL SPACE
         *
         *      mychar_ioctl()
         *          |
         *          v
         *      copy_from_user()
         *          |
         *          v
         *      my_value = 100
         */
        if (ioctl(fd,
                  SET_VALUE,
                  &value) < 0) {

                perror("ioctl SET_VALUE");

                close(fd);

                return EXIT_FAILURE;
        }


        printf("SET_VALUE successful\n");


        /* =================================================================
         *                         GET_VALUE
         * ================================================================= */


        /*
         * Clear our local variable.
         *
         * This is only to demonstrate that the driver
         * actually provides the value.
         */
        value = 0;


        printf("\n");
        printf("Requesting value from driver...\n");


        /*
         * GET_VALUE:
         *
         *
         *      USER SPACE
         *
         *      value
         *        ^
         *        |
         *        | &value
         *        |
         *      ioctl()
         *        |
         *        v
         *
         *      KERNEL SPACE
         *
         *      mychar_ioctl()
         *        |
         *        v
         *      my_value
         *        |
         *        v
         *      copy_to_user()
         *
         *
         * After ioctl() returns:
         *
         *      value == value stored in driver
         */
        if (ioctl(fd,
                  GET_VALUE,
                  &value) < 0) {

                perror("ioctl GET_VALUE");

                close(fd);

                return EXIT_FAILURE;
        }


        printf("GET_VALUE successful\n");

        printf("Value received from driver: %d\n",
               value);


        /* -----------------------------------------------------------------
         * STEP 3
         *
         * Close device.
         * ----------------------------------------------------------------- */

        close(fd);


        printf("\nDevice closed\n");


        return EXIT_SUCCESS;
}
