#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "myled_ioctl.h"

#define DEVICE "/dev/my_led"

int main(void)
{
    int fd, ret, state;

    printf("=== myled ioctl test ===\n\n");

    /* Open device */
    fd = open(DEVICE, O_RDWR);
    if(fd < 0) {
        perror("open failed");
        return 1;
    }
    printf("[1] Device opened\n");

    /* Test 1 — LED ON via ioctl */
    ret = ioctl(fd, MYLED_LED_ON);
    printf("[2] ioctl MYLED_LED_ON    ret=%d\n", ret);
    sleep(1);

    /* Test 2 — GET STATE */
    ret = ioctl(fd, MYLED_GET_STATE, &state);
    printf("[3] ioctl MYLED_GET_STATE ret=%d state=%d\n", ret, state);

    /* Test 3 — LED OFF */
    ret = ioctl(fd, MYLED_LED_OFF);
    printf("[4] ioctl MYLED_LED_OFF   ret=%d\n", ret);
    sleep(1);

    /* Test 4 — GET STATE again */
    ret = ioctl(fd, MYLED_GET_STATE, &state);
    printf("[5] ioctl MYLED_GET_STATE ret=%d state=%d\n", ret, state);

    /* Test 5 — TOGGLE 3 times */
    printf("[6] toggling LED 3 times...\n");
    int i;
    for(i = 0; i < 3; i++) {
        ret = ioctl(fd, MYLED_LED_TOGGLE);
        ioctl(fd, MYLED_GET_STATE, &state);
        printf("    toggle %d → LED=%d\n", i+1, state);
        sleep(1);
    }

    /* Test 6 — GET BUTTON STATE */
    ret = ioctl(fd, MYLED_GET_BTN, &state);
    printf("[7] ioctl MYLED_GET_BTN   ret=%d btn=%d\n", ret, state);

    /* Test 7 — RESET COUNTER */
    ret = ioctl(fd, MYLED_RESET_COUNT);
    printf("[8] ioctl MYLED_RESET_COUNT ret=%d\n", ret);

    /* Test 8 — read() to confirm final state */
    close(fd);
    fd = open(DEVICE, O_RDONLY);
    char buf[64];
    ret = read(fd, buf, sizeof(buf));
    buf[ret] = '\0';
    printf("[9] read() final state: %s", buf);

    close(fd);
    printf("[10] Device closed\n");
    printf("\n=== test complete ===\n");
    return 0;
}
