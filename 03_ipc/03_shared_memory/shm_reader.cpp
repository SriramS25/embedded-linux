//reader.cpp
//IPC_Mechanism -> sharedmemory

#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>
#include <unistd.h>

typedef struct Sensor_Data{
	int temp;
	int pressure;
	float hum;
}Sensor_Data_t;

int main(){

	int fd = shm_open("/my_shared_mem", O_RDONLY, 0666);
	
	Sensor_Data_t *shared = (Sensor_Data_t *)mmap(
			NULL,
			4096,
			PROT_READ,
			MAP_SHARED,
			fd,
			0);

	std::cout<<" Value obtained from reading the shared mem space are "<<std::endl;
	std::cout << "TEMP : " << shared->temp <<std::endl;
	std::cout << "PRES : " << shared->pressure << std::endl;
	std::cout << "HUMI : " << shared->hum << std::endl;

	munmap(shared,4096);
	close(fd);

	shm_unlink("/my_shared_mem");
}

