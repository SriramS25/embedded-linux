//writer.cpp
//IPC - shared memory
//
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/mman.h>
#include <iostream>
#include <sys/stat.h>

typedef struct Sensor_Data{
	int temp;
	int pressure;
	float hum;
}Sensor_Data_t;

int main(){
	
	const char *file_name = "/my_shared_mem";
	int fd = shm_open(file_name, O_CREAT | O_RDWR, 0666);

	if(fd == -1){
		perror("shm_open Error!!\n");
		return 1;
	}

	ftruncate(fd, 4096);

	Sensor_Data_t *shared = (Sensor_Data_t *)mmap(
			NULL,
			4096,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			0);
	if(shared== MAP_FAILED){
		perror("mmap failed!!\n");
		return 1;
	}

	//Sensor_Data_t *shared = (Sensor_Data_t *)ptr;
	
	std::cout<<"Enter the Temperature : ";
	std::cin >> shared->temp;
       	
	std::cout<<"Enter the pressure : ";
	std::cin >> shared->pressure;

	std::cout<<"Enter the humidity : ";
	std::cin >> shared->hum;

	munmap(shared, 4096);
	close(fd);

	return 0;
}	
