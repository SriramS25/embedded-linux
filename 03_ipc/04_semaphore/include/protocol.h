#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define SHM_NAME "/sensor_shm"
#define SEM_NAME "/sensor_sem"

#define MAGIC_NUMBER 0xABCD1234
#define VERSION      1

typedef struct{
	uint32_t magic;
	uint32_t version;

	int temperature;
	int pressure;
	float humidity;

}SensorData_t;

#endif


