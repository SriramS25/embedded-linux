#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/printk.h>

int value;
char *name;
int arr[4];
int arr_count;
int cb_value = 0;


/*-------------------------------------------------------------*/
/* Normal module parameters                                    */
/*-------------------------------------------------------------*/

module_param(value, int, 0644);
MODULE_PARM_DESC(value, "An integer value");

module_param(name, charp, 0644);
MODULE_PARM_DESC(name, "A string parameter");

module_param_array(arr, int, &arr_count, 0644);
MODULE_PARM_DESC(arr, "Integer array");


/*-------------------------------------------------------------*/
/* Callback function                                           */
/*-------------------------------------------------------------*/

int notify_param(const char *val, const struct kernel_param *kp)
{
    int res;

    pr_info("================================================\n");
    pr_info("[CB] notify_param() called\n");

    pr_info("[CB] Parameter name = %s\n", kp->name);
    pr_info("[CB] New value received as string = %s\n", val);

    pr_info("[CB] cb_value BEFORE param_set_int() = %d\n",
            cb_value);

    res = param_set_int(val, kp);

    pr_info("[CB] param_set_int() returned = %d\n", res);

    if (res == 0) {
        pr_info("[CB] cb_value AFTER param_set_int() = %d\n",
                cb_value);
    } else {
        pr_info("[CB] ERROR: parameter conversion failed\n");
    }

    pr_info("================================================\n");

    return res;
}


/*-------------------------------------------------------------*/
/* Parameter operations                                        */
/*-------------------------------------------------------------*/

const struct kernel_param_ops my_param_ops = {
    .set = notify_param,
    .get = param_get_int,
};


/*-------------------------------------------------------------*/
/* Register callback parameter                                 */
/*-------------------------------------------------------------*/

module_param_cb(cb_value, &my_param_ops, &cb_value, 0644);


/*-------------------------------------------------------------*/
/* Module initialization                                       */
/*-------------------------------------------------------------*/

static int __init hello_init(void)
{
    int i;

    pr_info("************************************************\n");
    pr_info("[INIT] hello_init() called\n");

    pr_info("[INIT] value = %d\n", value);

    pr_info("[INIT] cb_value = %d\n", cb_value);

    pr_info("[INIT] name = %s\n", name ? name : "(null)");

    pr_info("[INIT] arr_count = %d\n", arr_count);

    for (i = 0; i < arr_count; i++)
        pr_info("[INIT] arr[%d] = %d\n", i, arr[i]);

    pr_info("[INIT] Kernel Module inserted successfully\n");

    pr_info("************************************************\n");

    return 0;
}


/*-------------------------------------------------------------*/
/* Module cleanup                                              */
/*-------------------------------------------------------------*/

static void __exit hello_exit(void)
{
    pr_info("[EXIT] hello_exit() called\n");
    pr_info("[EXIT] Kernel Module removed successfully\n");
}


module_init(hello_init);
module_exit(hello_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sriram S");
MODULE_DESCRIPTION("Module parameters and callback demonstration");
MODULE_VERSION("1.0");
