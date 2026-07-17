#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/gpio.h> 	//gpio_request, gpio_set_value
#include <linux/interrupt.h>	//request_irq, IRQ flags
#include <linux/string.h>
#include "myled_ioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sriram");
MODULE_DESCRIPTION("GPIO LED + Button character driver");
MODULE_VERSION("1.0");

//------------------ GPIO PIN definitions (BCM numbering)
#define GPIO_BASE 512
#define LED_GPIO 529  // BCM17 GPIO17 -> 512 + 17
#define BTN_GPIO 539 // BCM27 -> 512 + 27

//----------------- Driver state
static int led_state = 0;	//LED 0=OFF 1=ON
static int btn_state = 0;	//0=released 1=pressed
static int irq_num   = 0;	//IRQ number for button
static int btn_press_count = 0; //how many times button pressed

//------------------ Char driver globals
static dev_t 		dev_num;
static struct cdev 	my_cdev;
static struct class	*my_class;
static struct device    *my_device;

/* ═══════════════════════════════════════
   INTERRUPT HANDLER — called on button edge
═══════════════════════════════════════ */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    btn_state = gpio_get_value(BTN_GPIO);
    if(btn_state == 0) {        // LOW = pressed (pull-up config)
        btn_press_count++;
        dev_info(my_device,
                 "IRQ! button pressed [count=%d]\n",
                 btn_press_count);
    } else {
        dev_info(my_device, "IRQ! button released\n");
    }
    return IRQ_HANDLED;         // tell kernel: IRQ handled by us
}

//---------------FOPS
static int my_open(struct inode *inode, struct file *file){
	dev_info(my_device, "open() - led=%d btn=%d called\n",led_state,btn_state);
	return 0;
}

static int my_release(struct inode *inode, struct file *file){
	dev_info(my_device,"close() - called\n");
	return 0;
}

static ssize_t my_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos){
       	char cmd[4];       
	int bytes;
	
	dev_info(my_device, "write() - called - count=%zu\n",count);

	bytes = min(count, sizeof(cmd)-1);

	if(copy_from_user(cmd, ubuf, bytes)){
		dev_err(my_device, "copy_from_user failed\n");
		return -EFAULT;
	}

	cmd[bytes] = '\0';
	dev_info(my_device, "write() received cmd=[%s]\n",cmd);

	//parse command
	if(cmd[0] == '1'){
		led_state  = 1;
		gpio_set_value(LED_GPIO, 1); //LED ON
		dev_info(my_device, "LED -> ON\n");
	}else if(cmd[0] == '0'){
		led_state = 0;
		gpio_set_value(LED_GPIO, 0); //LED OFF
		dev_info(my_device, "LED -> OFF\n");
	}else{
		dev_warn(my_device, "unknow cmd [%c] - use '1' or '0'\n", cmd[0]);
		return -EINVAL;
	}
	return bytes;
}

static ssize_t my_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos){
	char state_str[32];
	int len;

	dev_info(my_device, "read() called\n");

	//ONly send once - EOF on second call
	if(*ppos > 0){
		return 0;
	}

	//build state string
	len = snprintf(state_str, sizeof(state_str), "LED:%d BTN%d PRESSES:%d\n",led_state,btn_state, btn_press_count);
	dev_info(my_device, "read() sending [%s]\n", state_str);

	if(copy_to_user(ubuf, state_str, len)){
		dev_err(my_device,"copy_to_user failed\n");
		return -EFAULT;
	}

	*ppos += len;
       	return len;
}

//-------------IOCTL
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	int val;
	dev_info(my_device, "ioctl() cmd=0x%x arg=%lu\n",cmd, arg);

	if(_IOC_TYPE(cmd) != MYLED_MAGIC){
		return -ENOTTY;
	}

	switch(cmd){

    case MYLED_LED_ON:
        led_state = 1;
        gpio_set_value(LED_GPIO, 1);
        dev_info(my_device, "ioctl: LED ON\n");
        break;

    case MYLED_LED_OFF:
        led_state = 0;
        gpio_set_value(LED_GPIO, 0);
        dev_info(my_device, "ioctl: LED OFF\n");
        break;

    case MYLED_LED_TOGGLE:
        led_state = !led_state;
        gpio_set_value(LED_GPIO, led_state);
        dev_info(my_device, "ioctl: LED TOGGLE → %d\n", led_state);
        break;

    case MYLED_GET_STATE:
        val = led_state;
        dev_info(my_device, "ioctl: GET_STATE → %d\n", val);
        if(copy_to_user((int __user *)arg, &val, sizeof(int)))
            return -EFAULT;
        break;

    case MYLED_GET_BTN:
        val = btn_state;
        dev_info(my_device, "ioctl: GET_BTN → %d\n", val);
        if(copy_to_user((int __user *)arg, &val, sizeof(int)))
            return -EFAULT;
        break;

    case MYLED_RESET_COUNT:
        btn_press_count = 0;
        dev_info(my_device, "ioctl: press count reset\n");
        break;

    default:
        return -ENOTTY;
    }

	return 0;
}


//------------fops table
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = my_read,
	.write = my_write,
	.unlocked_ioctl = my_ioctl,
	.open = my_open,
	.release = my_release,
};

///-----------------INIT
static int __init myled_init(void){
	int ret;
	pr_info("myled: initializing\n");
	pr_info("myled: LED_GPIO=%d BTN_GPIO=%d\n", LED_GPIO, BTN_GPIO);
        pr_info("myled: kernel GPIO base = %d\n", GPIO_BASE);

	//step1: request LED GPIO
	ret = gpio_request(LED_GPIO, "myled_led");
	if(ret < 0){
		pr_err("myled: gpio_request LED failed\n");
		return ret;
	}

	gpio_direction_output(LED_GPIO,0); //OUTPUT, starts LOW
	pr_info("myled: LED GPIO%d configured as OUTPUT\n", LED_GPIO);


	//step 2: request button GPIO
	ret = gpio_request(BTN_GPIO, "myled_btn");
	if(ret <0){
		pr_err("myled: gpio_request BTN failed\n");
		goto err_btn_gpio;
	}

	gpio_direction_input(BTN_GPIO);
	pr_info("myled: BTN GPIO%d configure as INPUT\n", BTN_GPIO);


	//step 3: GET IRQ number for button
	irq_num = gpio_to_irq(BTN_GPIO);
	if(irq_num < 0){
		pr_err("myled: gpio_to_irq failed\n");
		ret = irq_num;
		goto err_irq;
	}

	pr_info("myled: BTN GPIO%d mapped ot IRQ%d\n", BTN_GPIO, irq_num);


	//step 4: Register interrupt handler
	ret = request_irq(irq_num, btn_irq_handler, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "my_led_btn", NULL);

	if(ret < 0){
		pr_err("my_led: request_irq failed\n");
		goto err_irq;
	}

	pr_info("myled: IRQ%d registered\n",irq_num);

	//step 5: Alloc major/minor number
	ret = alloc_chrdev_region(&dev_num, 0,1,"myled");
	if(ret < 0){
		pr_err("myled: alloc_chrdev_region failed\n");
		goto err_chrdev;
	}

	pr_info("myled: major=%d minor=%d\n", MAJOR(dev_num), MINOR(dev_num));


	//step 6: cdev init + add
	cdev_init(&my_cdev, &fops);
	my_cdev.owner = THIS_MODULE;

	ret = cdev_add(&my_cdev, dev_num, 1);

	if(ret < 0){
		pr_err("myled: cdev_add failed\n");
		goto err_cdev;
	}


	//step 7: create class
	my_class = class_create("myled");
	if(IS_ERR(my_class)){
		pr_err("myled: classs create failed\n");
		ret = PTR_ERR(my_class);
		goto err_class;
	}

	//step 8: Create device node $$
	my_device = device_create(my_class, NULL, dev_num, NULL,"my_led");
	if(IS_ERR(my_device)){
		pr_err("myled: device create failed \n");
		ret = PTR_ERR(my_device);
		goto err_device;
	}

	dev_info(my_device, "ready at /dev/myled\n");
	dev_info(my_device, "LED=GPIO%d BTN=GPIO%d IRQ=%d\n",
             LED_GPIO, BTN_GPIO, irq_num);

	return 0;


/* ── error unwinding ── */
err_device:
    class_destroy(my_class);
err_class:
    cdev_del(&my_cdev);
err_cdev:
    unregister_chrdev_region(dev_num, 1);
err_chrdev:
    free_irq(irq_num, NULL);
err_irq:
    gpio_free(BTN_GPIO);
err_btn_gpio:
    gpio_free(LED_GPIO);
    return ret;
}

//---------------------EXIT
static void __exit myled_exit(void){
	device_destroy(my_class, dev_num);
	class_destroy(my_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(dev_num, 1);
	free_irq(irq_num, NULL);
	gpio_set_value(LED_GPIO, 0);
	gpio_free(BTN_GPIO);
	gpio_free(LED_GPIO);
	pr_info("myled: removed — LED turned off\n");
}


module_init(myled_init);
module_exit(myled_exit);













