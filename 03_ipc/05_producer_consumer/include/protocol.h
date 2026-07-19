#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define SHM_NAME "/pc_buffer"

#define SEM_MUTEX "/pc_mutex"
#define SEM_EMPTY "/pc_empty"
#define SEM_FULL  "/pc_full"

#define BUFFER_SIZE 5

typedef struct
{
    int data[BUFFER_SIZE];

    int head;

    int tail;

} SharedBuffer;

#endif
