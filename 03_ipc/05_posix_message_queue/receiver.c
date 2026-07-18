#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>

#include "protocol.h"

int main()
{
    mqd_t mq;

    mq = mq_open(MQ_NAME,
                 O_RDONLY);

    if(mq == (mqd_t)-1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    IPCPacket packet;

    unsigned int priority;

    ssize_t bytes =
        mq_receive(mq,
                   (char *)&packet,
                   sizeof(packet),
                   &priority);

    if(bytes == -1)
    {
        perror("mq_receive");
        exit(EXIT_FAILURE);
    }

    if(packet.magic != MAGIC_NUMBER)
    {
        printf("Invalid Packet\n");
        exit(EXIT_FAILURE);
    }

    printf("\nPacket Received\n");
    printf("---------------------------\n");

    printf("Priority : %u\n",priority);
    printf("Magic    : 0x%X\n",packet.magic);
    printf("Version  : %u\n",packet.version);
    printf("Type     : %u\n",packet.type);
    printf("Length   : %u\n",packet.len);
    printf("Payload  : %s\n",packet.payload);

    mq_close(mq);

    return 0;
}
