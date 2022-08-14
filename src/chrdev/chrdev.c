/* Code taken from https://stepik.org/course/2051/syllabus */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>

static dev_t first;
static unsigned int count = 1;
static int my_major = 700, my_minor = 0;
static struct cdev *my_cdev;

#define MYDEV_NAME "mychrdev"
#define KBUF_SIZE (size_t)((10) * PAGE_SIZE)

static struct class *my_device_class;  // device class

static int mychrdev_open(struct inode *inode, struct file *file)
{
    static int counter = 0;

    char *kbuf = kmalloc(KBUF_SIZE, GFP_KERNEL);
    file->private_data = kbuf;

    printk(KERN_INFO "Opening device %s\n\n", MYDEV_NAME);
    counter++;

    printk(KERN_INFO "kbuf = %p, counter = %d, mod refcounter = %d\n", kbuf, counter, module_refcount(THIS_MODULE));
    return 0;
}

static int mychrdev_release(struct inode *inode, struct file *file)
{
    char *kbuf = file->private_data;
    printk(KERN_INFO "Releasing device %s\n\n", MYDEV_NAME);

    printk(KERN_INFO "Free kbuf = %p", kbuf);
    if (kbuf)
    {
        kfree(kbuf);
        kbuf = file->private_data = NULL;
    }

    return 0;
}

static ssize_t mychrdev_read(struct file *file,
                             char __user *buf,          // parm came from user space
                             size_t lbuf, loff_t *ppos) // long offset
{
    char *kbuf = file->private_data;
    ssize_t nbytes = lbuf - copy_to_user(buf, kbuf + *ppos, lbuf); // copy to user space storage
    *ppos += nbytes;

    printk(KERN_INFO "Reading device %s - nbytes = %ld, ppos = %lld\n\n", MYDEV_NAME, nbytes, *ppos);

    return nbytes;
}

static ssize_t __exit mychrdev_write(struct file *file,
                                     const char __user *buf, // parm came from user space
                                     size_t lbuf, loff_t *ppos)
{
    char *kbuf = file->private_data;
    ssize_t nbytes = lbuf - copy_from_user(kbuf + *ppos, buf, lbuf);
    *ppos += nbytes;

    printk(KERN_INFO "Writing device %s - nbytes = %ld, ppos = %lld \n\n", MYDEV_NAME, nbytes, *ppos);

    return nbytes;
}

static loff_t mychrdev_lseek(struct file *file, loff_t offset, int orig)
{
    loff_t testpos;
    switch (orig)
    {
    case SEEK_SET:
        testpos = offset;
        break;
    case SEEK_CUR:
        testpos = file->f_pos + offset;
        break;
    case SEEK_END:
        testpos = KBUF_SIZE + offset;
        break;
    default:
        return -EINVAL; // all errors needs to be inverted on return
    }

    testpos = testpos < KBUF_SIZE ? testpos : KBUF_SIZE;
    testpos = testpos >= 0 ? testpos : 0;
    file->f_pos = testpos;

    printk(KERN_INFO "Seeking to %lld position\n", testpos);

    return testpos;
}

static const struct file_operations mycdev_fops = {
    .owner = THIS_MODULE,
    .read = mychrdev_read,
    .write = mychrdev_write,
    .open = mychrdev_open,
    .release = mychrdev_release,
    .llseek = mychrdev_lseek};

static int __init init_chrdev(void)
{
    printk(KERN_INFO "Hello, loading");

    first = MKDEV(my_major, my_minor);
    register_chrdev_region(first, count, MYDEV_NAME);

    my_cdev = cdev_alloc();

    cdev_init(my_cdev, &mycdev_fops); // file operations instance
    cdev_add(my_cdev, first, count);

    my_device_class = class_create(THIS_MODULE, "my_device_class");
    device_create(my_device_class, NULL, first, "%s", "my_chrdev");

    printk(KERN_INFO "Created device class %s", MYDEV_NAME);

    return 0;
}

static void __exit cleanup_chrdev(void)
{
    printk(KERN_INFO "Leaving");

    device_destroy(my_device_class, first);
    class_destroy(my_device_class);

    if (my_cdev)
        cdev_del(my_cdev);

    unregister_chrdev_region(first, count);
}

module_init(init_chrdev);
module_exit(cleanup_chrdev);

MODULE_LICENSE("GPL");
