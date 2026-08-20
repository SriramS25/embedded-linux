#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>

#define PROC_NAME "mychar_status"

static struct proc_dir_entry *proc_entry;

static ssize_t my_proc_read( struct file *file,
			     char __user *buf,
			     size_t len,
			     loff_t *offset)
{
	char msg[] = "mychar_driver: READY\n";
	size_t msg_len = strlen(msg);
	pr_info("[my_procfs]: read() called\n");

	if(*offset >= msg_len){
		pr_info("[my_procfs]: EOF reached\n");
		return 0;
	}

	if(len > msg_len - *offset){
		len = msg_len - *offset;
		if(copy_to_user(buf, msg + *offset, len)){
			pr_err("[my_procfs]: "
                       "copy_to_user() failed\n");
		return -EFAULT;
		}
	
	*offset += len;
	 pr_info("[my_procfs]: "
                "%zu bytes sent to userspace\n",
                len);

	}
        return len;

}

static const struct proc_ops my_proc_ops = {
	.proc_read = my_proc_read,
};

static int __init my_procfs_init(void){
	pr_info("[my_procfs]: "
                "Module initialization started\n");
	proc_entry = proc_create(PROC_NAME,
				 0444,
				 NULL,
				 &my_proc_ops);

	if(!proc_entry){
		pr_err("[my_procfs]: proc_create() failed\n");
		return -ENOMEM;
	}

	pr_info("[my_procfs]: /proc/%s created\n", PROC_NAME);
	return 0;
}

static void __exit my_procfs_exit(void){
	proc_remove(proc_entry);
	pr_info("[my_procfs]: "
                "/proc/%s removed\n",
                PROC_NAME);
}

module_init(my_procfs_init);
module_exit(my_procfs_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sriram S");
MODULE_DESCRIPTION("Educational procfs example");
MODULE_VERSION("1.0");


