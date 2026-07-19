#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define MQ_NAME "/printer_queue"

#define MAGIC_NUMBER   0xABCD1234
#define VERSION        1

#define SENSOR_DATA    1
#define MOTOR_DATA     2
#define ERROR_DATA     3

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t type;
    uint32_t len;

    char payload[256];

} IPCPacket;

#endif
