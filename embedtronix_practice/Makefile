#obj-m += 01_sample_driver.o
#obj-m += 02_driver_with_args.o
#obj-m += 03_major_num_static_alloc.o
#obj-m += 04_major_num_dynamic_alloc.o
obj-m += 05_dev_create.o


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
