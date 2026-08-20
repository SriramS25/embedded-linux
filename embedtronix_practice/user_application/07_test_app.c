/*
 * ============================================================================
 *                         USERSPACE TEST APPLICATION
 * ============================================================================
 *
 * File        : 07_test_app.c
 * Author      : Sriram S
 *
 * Description :
 *     Test application for the educational Linux character device driver.
 *
 * Device:
 *
 *     /dev/mychar_dev
 *
 * Operations:
 *
 *     1. Write data to driver
 *     2. Read data from driver
 *     3. Exit
 * ============================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>      /* open() */
#include <unistd.h>     /* read(), write(), close() */
#include <errno.h>      /* errno */
#include <stdint.h>     /* uint8_t */


/* Device file created by the kernel driver */
#define DEVICE_PATH "/dev/mychar_dev"

/* Must match driver buffer size for this test */
#define BUFFER_SIZE 1024


int main(void)
{
        int fd;
        int choice;

        char write_buf[BUFFER_SIZE];
        char read_buf[BUFFER_SIZE];


        /*
         * Open the character device.
         *
         * O_RDWR:
         *
         *      Open for both reading and writing.
         */
        fd = open(DEVICE_PATH, O_RDWR);

        if (fd < 0) {

                perror("open");

                return EXIT_FAILURE;
        }


        printf("\n");
        printf("========================================\n");
        printf("      MY CHARACTER DRIVER TEST APP\n");
        printf("========================================\n");


        while (1) {

                printf("\n");
                printf("1. Write\n");
                printf("2. Read\n");
                printf("3. Exit\n");
                printf("Enter choice: ");


                if (scanf("%d", &choice) != 1) {

                        printf("Invalid input\n");

                        /*
                         * Clear invalid input.
                         */
                        while (getchar() != '\n')
                                ;

                        continue;
                }


                /*
                 * Remove newline left by scanf().
                 */
                while (getchar() != '\n')
                        ;


                switch (choice) {


                /* =========================================================
                 * WRITE
                 * ======================================================= */

                case 1:
                {
                        ssize_t bytes_written;


                        printf("Enter data: ");


                        if (fgets(write_buf,
                                  sizeof(write_buf),
                                  stdin) == NULL) {

                                printf("Failed to read input\n");

                                break;
                        }


                        /*
                         * Remove trailing newline.
                         */
                        write_buf[strcspn(write_buf, "\n")] = '\0';


                        /*
                         * Include terminating '\0' in the data written.
                         *
                         * This makes it convenient to print the data
                         * as a C string during this educational exercise.
                         */
                        bytes_written = write(fd,
                                              write_buf,
                                              strlen(write_buf) + 1);


                        if (bytes_written < 0) {

                                perror("write");

                                break;
                        }


                        printf("Write successful\n");
                        printf("Bytes written = %zd\n",
                               bytes_written);

                        break;
                }


                /* =========================================================
                 * READ
                 * ======================================================= */

                case 2:
                {
                        ssize_t bytes_read;


                        /*
                         * Clear the read buffer.
                         */
                        memset(read_buf, 0, sizeof(read_buf));


                        bytes_read = read(fd,
                                          read_buf,
                                          sizeof(read_buf) - 1);


                        if (bytes_read < 0) {

                                perror("read");

                                break;
                        }


                        if (bytes_read == 0) {

                                printf("No data available / EOF\n");

                                break;
                        }


                        /*
                         * Ensure NULL termination.
                         *
                         * Our driver stores the '\0' because the
                         * write operation includes strlen()+1.
                         */
                        read_buf[bytes_read] = '\0';


                        printf("Read successful\n");
                        printf("Bytes read = %zd\n",
                               bytes_read);

                        printf("Data = %s\n",
                               read_buf);

                        break;
                }


                /* =========================================================
                 * EXIT
                 * ======================================================= */

                case 3:

                        close(fd);

                        printf("Application exiting\n");

                        return EXIT_SUCCESS;


                default:

                        printf("Invalid option\n");

                        break;
                }
        }


        close(fd);

        return EXIT_SUCCESS;
}
