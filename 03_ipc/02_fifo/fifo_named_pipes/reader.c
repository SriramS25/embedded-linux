#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "protocol.h"

int main()
{
    if(mkfifo(FIFO_PATH,0666)==-1)
    {
        if(errno!=EEXIST)
        {
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
    }

    printf("Waiting for writer...\n");

    int fd=open(FIFO_PATH,O_RDONLY);

    if(fd==-1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    IPCPacket packet;

    ssize_t ret=read(fd,
                     &packet,
                     sizeof(packet));

    if(ret<=0)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    if(packet.magic!=MAGIC_NUMBER)
    {
        printf("Invalid Packet!\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("\nPacket Received\n");
    printf("------------------------\n");
    printf("Magic   : 0x%X\n",packet.magic);
    printf("Version : %u\n",packet.version);
    printf("Type    : %u\n",packet.type);
    printf("Length  : %u\n",packet.len);
    printf("Payload : %s\n",packet.payload);

    close(fd);

    return 0;
}
