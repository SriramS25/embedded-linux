#ifndef MYLED_IOCTL_H
#define MYLED_IOCTL_H

#include <linux/ioctl.h>

#define MYLED_MAGIC 'L'

#define MYLED_LED_ON		_IO(MYLED_MAGIC, 0) //turn LED on
#define MYLED_LED_OFF 		_IO(MYLED_MAGIC, 1) //turn LED off
#define MYLED_LED_TOGGLE	_IO(MYLED_MAGIC, 2) //toggle LED
#define MYLED_GET_STATE		_IOR(MYLED_MAGIC, 3, int) //get LED state
#define MYLED_GET_BTN		_IOR(MYLED_MAGIC, 4, int) //get button state
#define MYLED_RESET_COUNT 	_IO(MYLED_MAGIC, 5) // reset press counter

#endif
