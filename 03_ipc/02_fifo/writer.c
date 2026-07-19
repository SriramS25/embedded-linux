#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "./include/protocol.h"

int main()
{
    /* Create FIFO if it doesn't exist */
    if(mkfifo(FIFO_PATH, 0666) == -1)
    {
        if(errno != EEXIST)
        {
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
    }

    printf("Waiting for reader...\n");

    int fd = open(FIFO_PATH, O_WRONLY);

    if(fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    IPCPacket packet = {0};

    packet.magic   = MAGIC_NUMBER;
    packet.version = VERSION;
    packet.type    = SENSOR_DATA;

    printf("Enter Sensor Message : ");

    fgets(packet.payload,
          sizeof(packet.payload),
          stdin);

    packet.payload[strcspn(packet.payload,"\n")] = '\0';

    packet.len = strlen(packet.payload);

    ssize_t ret = write(fd,
                        &packet,
                        sizeof(packet));

    if(ret != sizeof(packet))
    {
        perror("write");
    }

    printf("\nPacket Sent\n");
    printf("Type    : %u\n",packet.type);
    printf("Length  : %u\n",packet.len);
    printf("Payload : %s\n",packet.payload);

    close(fd);

    return 0;
}
