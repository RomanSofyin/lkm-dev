/* Code taken from https://stepik.org/course/2051/syllabus */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>

static char *kbuf;
static dev_t first;
static unsigned int coont = 1;
static int my_major = 700, my_minor = 0;
static struct cdev *my_cdev;

#define MYDEV_NAME "mychrdev"
#define KBUF_SIZE size_t ((10) * PAGE_SIZE)

static int __init init_chrdev(void)
{
    printk(KERN_INFO "Hello, loading");
    return 0;
}

static void __exit cleanup_chrdev(void)
{
    printk(KERN_INFO "Leaving");
}

module_init(init_chrdev);
module_exit(cleanup_chrdev);

MODULE_LICENSE("GPL");
