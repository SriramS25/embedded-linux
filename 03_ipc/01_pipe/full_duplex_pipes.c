#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAGIC_NUMBER   0xABCD1234
#define VERSION        1

#define SENSOR_DATA    1
#define MOTOR_DATA     2

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t type;
    uint32_t len;
    char payload[256];

} IPCPacket;

ssize_t write_exact(int fd, const void *buf, size_t len)
{
    size_t total = 0;

    while(total < len)
    {
        ssize_t written = write(fd,
                                (const char *)buf + total,
                                len - total);

        if(written < 0)
        {
            if(errno == EINTR)
                continue;

            return -1;
        }

        total += written;
    }

    return total;
}

ssize_t read_exact(int fd, void *buf, size_t len)
{
    size_t total = 0;

    while(total < len)
    {
        ssize_t n = read(fd,
                         (char *)buf + total,
                         len - total);

        if(n == 0)
            return 0;

        if(n < 0)
        {
            if(errno == EINTR)
                continue;

            return -1;
        }

        total += n;
    }

    return total;
}

int main()
{
    int parent_to_child[2];
    int child_to_parent[2];

    if(pipe(parent_to_child) == -1 ||
       pipe(child_to_parent) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if(pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    /*========================= PARENT =========================*/

    if(pid > 0)
    {
        close(parent_to_child[0]);      // Parent doesn't read Pipe1
        close(child_to_parent[1]);      // Parent doesn't write Pipe2

        IPCPacket tx_packet = {0};
        IPCPacket rx_packet = {0};

        tx_packet.magic = MAGIC_NUMBER;
        tx_packet.version = VERSION;
        tx_packet.type = SENSOR_DATA;

        printf("\nEnter message for child : ");

        fgets(tx_packet.payload,
              sizeof(tx_packet.payload),
              stdin);

        tx_packet.payload[strcspn(tx_packet.payload, "\n")] = '\0';

        tx_packet.len = strlen(tx_packet.payload);

        printf("\n[PARENT] Sending Packet\n");
        printf("PID      : %d\n", getpid());
        printf("Type     : %u\n", tx_packet.type);
        printf("Length   : %u\n", tx_packet.len);
        printf("Payload  : %s\n\n", tx_packet.payload);

        if(write_exact(parent_to_child[1],
                       &tx_packet,
                       sizeof(tx_packet))
                != sizeof(tx_packet))
        {
            perror("write_exact");
            exit(EXIT_FAILURE);
        }

        if(read_exact(child_to_parent[0],
                      &rx_packet,
                      sizeof(rx_packet))
                != sizeof(rx_packet))
        {
            perror("read_exact");
            exit(EXIT_FAILURE);
        }

        printf("\n[PARENT] Reply Received\n");
        printf("PID      : %d\n", getpid());
        printf("Type     : %u\n", rx_packet.type);
        printf("Length   : %u\n", rx_packet.len);
        printf("Payload  : %s\n\n", rx_packet.payload);

        close(parent_to_child[1]);
        close(child_to_parent[0]);

        wait(NULL);
    }

    /*========================= CHILD =========================*/

    else
    {
        close(parent_to_child[1]);      // Child doesn't write Pipe1
        close(child_to_parent[0]);      // Child doesn't read Pipe2

        IPCPacket rx_packet = {0};
        IPCPacket tx_packet = {0};

        if(read_exact(parent_to_child[0],
                      &rx_packet,
                      sizeof(rx_packet))
                != sizeof(rx_packet))
        {
            perror("read_exact");
            exit(EXIT_FAILURE);
        }

        if(rx_packet.magic != MAGIC_NUMBER)
        {
            printf("Invalid Packet!\n");
            exit(EXIT_FAILURE);
        }

        printf("\n[CHILD] Packet Received\n");
        printf("PID      : %d\n", getpid());
        printf("Type     : %u\n", rx_packet.type);
        printf("Length   : %u\n", rx_packet.len);
        printf("Payload  : %s\n\n", rx_packet.payload);

        tx_packet.magic = MAGIC_NUMBER;
        tx_packet.version = VERSION;
        tx_packet.type = MOTOR_DATA;

        printf("Enter reply for Parent : ");

        fgets(tx_packet.payload,
              sizeof(tx_packet.payload),
              stdin);

        tx_packet.payload[strcspn(tx_packet.payload, "\n")] = '\0';

        tx_packet.len = strlen(tx_packet.payload);

        if(write_exact(child_to_parent[1],
                       &tx_packet,
                       sizeof(tx_packet))
                != sizeof(tx_packet))
        {
            perror("write_exact");
            exit(EXIT_FAILURE);
        }

        close(parent_to_child[0]);
        close(child_to_parent[1]);
    }

    return 0;
} 
