#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "mydev_ioctl.h"   // shared header

#define DEVICE "/dev/mydev"

int main(void)
{
    int fd, ret, len;
    char buf[256];
    mydev_stats_t stats;

    printf("=== mydev ioctl test ===\n\n");

    /* Open device */
    fd = open(DEVICE, O_RDWR);
    if(fd < 0) {
        perror("open failed");
        return 1;
    }
    printf("[1] Device opened\n");

    /* Write data */
    ret = write(fd, "Hello from VOLT", 15);
    printf("[2] write() returned %d\n", ret);

    /* Get buffer length */
    ret = ioctl(fd, MYDEV_GET_LEN, &len);
    printf("[3] MYDEV_GET_LEN: buf_len = %d\n", len);

    /* Get stats */
    ret = ioctl(fd, MYDEV_GET_STATS, &stats);
    printf("[4] MYDEV_GET_STATS:\n");
    printf("      buf_size    = %d\n", stats.buf_size);
    printf("      buf_len     = %d\n", stats.buf_len);
    printf("      read_count  = %d\n", stats.read_count);
    printf("      write_count = %d\n", stats.write_count);

    /* Read data back */
    //lseek(fd, 0, SEEK_SET);
    close(fd);
    fd = open(DEVICE, O_RDONLY);
    ret = read(fd, buf, sizeof(buf));
    buf[ret] = '\0';
    printf("[5] read() returned %d: [%s]\n", ret, buf);

    /* Resize buffer to 512 bytes */
    int new_size = 512;
    ret = ioctl(fd, MYDEV_SET_SIZE, &new_size);
    printf("[6] MYDEV_SET_SIZE to 512: ret=%d\n", ret);

    /* Verify new size via stats */
    ret = ioctl(fd, MYDEV_GET_STATS, &stats);
    printf("[7] After resize — buf_size = %d\n", stats.buf_size);

    /* Clear buffer */
    ret = ioctl(fd, MYDEV_CLEAR);
    printf("[8] MYDEV_CLEAR: ret=%d\n", ret);

    /* Verify clear */
    ret = ioctl(fd, MYDEV_GET_LEN, &len);
    printf("[9] After clear — buf_len = %d\n", len);

    /* Final stats */
    ret = ioctl(fd, MYDEV_GET_STATS, &stats);
    printf("[10] Final stats:\n");
    printf("      buf_size    = %d\n", stats.buf_size);
    printf("      buf_len     = %d\n", stats.buf_len);
    printf("      read_count  = %d\n", stats.read_count);
    printf("      write_count = %d\n", stats.write_count);

    close(fd);
    printf("\n[11] Device closed\n");
    printf("=== test complete ===\n");
    return 0;
}
