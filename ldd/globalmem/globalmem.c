/*
* file : globalmem.c
* desc : a global momery device for linux device driver development learning
* auth : Gavin Ding
* vers : 1.0.1
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/device.h>

#undef OK
#define OK 0

#define UNUSED(x) ((x) = (x))

#define ALERT(fmt...) printk(KERN_ALERT fmt)
#define CHECK_RETURN(opt, ret)\
do {\
    if (opt)\
    {\
        ALERT("%s-%d: ret=%d.\n", __FUNCTION__, __LINE__, ret);\
        return(ret);\
    }\
}\
while(0)

#define DEV_NAME        "globalmem"         /* device name */
#define GLOBALMEM_SIZE  0x1000              /* global memory size, 4KB */

#define GLOBALMEM_MAGIC 'G'
#define MEM_CLEAR _IO(GLOBALMEM_MAGIC, 0)   /* clear global memory cmd word */


/* globalmem device struct */
struct globalmem_dev 
{
    dev_t            dev_num;               /* device number in kernel */
    char*            dev_name;              /* device name */
    struct cdev      cdev;                  /* cdev structure */
    struct class    *mem_class;             /* device class */
    unsigned char    mem[GLOBALMEM_SIZE];   /* global memory */
    struct semaphore sem;                   /* semaphore for concurrency control */
};

struct globalmem_dev *globalmem_devp;       /* device struct pointer */

int globalmem_open(struct inode *inodp,
                   struct file *filp)
{
    struct globalmem_dev *devp;
    
    devp = container_of(inodp->i_cdev, struct globalmem_dev, cdev);
    filp->private_data = devp;

    return OK;
}

int globalmem_release(struct inode *inodp,
                      struct file *filp)
{
    UNUSED(inodp);
    UNUSED(filp);

    return OK;
}

static ssize_t globalmem_read(struct file *filp,
                              char __user *buf,
                              size_t count,
                              loff_t *ppos)
{
    int                   ret  = 0;
    unsigned long         p    = *ppos;
    struct globalmem_dev *devp = (struct globalmem_dev*)filp->private_data;

    CHECK_RETURN(p >= GLOBALMEM_SIZE, 0);
    CHECK_RETURN(NULL == devp, 0);

    if (count > GLOBALMEM_SIZE - p)
    {
        count = GLOBALMEM_SIZE - p;
    }
    
    if (down_interruptible(&(devp->sem)))
    {
        return -ERESTARTSYS;
    }
    if (copy_to_user(buf, (void*)(devp->mem + p), count))
    {
        ret = -EFAULT;
    }
    else
    {
        *ppos += count;
        ret = count;

        printk(KERN_INFO "read %d byte(s) from %lu\n", count, p);
    }
    up(&(devp->sem));

    return ret;
}

static ssize_t globalmem_write(struct file *filp,
                               const char __user *buf,
                               size_t count,
                               loff_t *ppos)
{
    int                   ret  = 0;
    unsigned long         p    = *ppos;
    struct globalmem_dev *devp = (struct globalmem_dev*)filp->private_data;

    CHECK_RETURN(p >= GLOBALMEM_SIZE, 0);
    CHECK_RETURN(NULL == devp, 0);

    if (count > GLOBALMEM_SIZE - p)
    {
        count = GLOBALMEM_SIZE - p;
    }

    if (down_interruptible(&(devp->sem)))
    {
        return -ERESTARTSYS;
    }
    if (copy_from_user(devp->mem + p, buf, count))
    {
        ret = -EFAULT;
    }
    else
    {
        *ppos += count;
        ret = count;

        printk(KERN_INFO "write %d byte(s) from %lu\n", count, p);
    }
    up(&(devp->sem));

    return ret;
}

typedef enum tagSeekPosition
{
    seek_head = 0,
    seek_curr = 1,

    seek_butt   
} seekPositionE;

static loff_t globalmem_llseek(struct file *filp,
                               loff_t offset,
                               int orig)
{
    loff_t                ret  = 0;
    struct globalmem_dev *devp = (struct globalmem_dev*)filp->private_data;

    CHECK_RETURN(NULL == devp, -EINVAL);

    if (down_interruptible(&(devp->sem)))
    {
        return -ERESTARTSYS;
    }
    switch (orig)
    {
        case seek_head:  /* from file head */
            if (offset < 0)
            {
                ret = -EINVAL;
                break;
            }

            if ((unsigned int)offset > GLOBALMEM_SIZE)
            {
                ret = -EINVAL;
                break;
            }

            filp->f_pos = (unsigned int)offset;
            ret = filp->f_pos;
            break;
        case seek_curr:  /* from current position */
            if ((filp->f_pos + offset) < 0)
            {
                ret = -EINVAL;
                break;
            }
            
            if ((unsigned int)(filp->f_pos + offset) > GLOBALMEM_SIZE)
            {
                ret = -EINVAL;
                break;
            }

            filp->f_pos += offset;
            ret = filp->f_pos;
            break;
        default:
            ret = -EINVAL;
            break;
    }
    up(&(devp->sem));

    return ret;
}

static long globalmem_ioctl(struct file *filp,
                            unsigned int cmd,
                            unsigned long arg)
{
    struct globalmem_dev *devp = (struct globalmem_dev*)filp->private_data;

    CHECK_RETURN(NULL == devp, 0);
    
    switch (cmd)
    {
        case MEM_CLEAR:
            if (down_interruptible(&(devp->sem)))
            {
                return -ERESTARTSYS;
            }
            memset(devp->mem, 0, GLOBALMEM_SIZE);
            up(&(devp->sem));
            
            printk(KERN_INFO "globalmem is set to zero\n");
            break;

        default:
            return -EINVAL;  /* other unsupported commands */
    }

    return 0;
}

static const struct file_operations globalmem_fops =
{
    .owner          = THIS_MODULE,
    .open           = globalmem_open,
    .release        = globalmem_release,
    .read           = globalmem_read,
    .write          = globalmem_write,
    .llseek         = globalmem_llseek,
    .unlocked_ioctl = globalmem_ioctl,
};

static int __init globalmem_setup_cdev(struct globalmem_dev *devp)
{
    int   err   = 0;
    dev_t devno = 0;

    CHECK_RETURN(NULL == devp, -EFAULT);
    
    /* init & add cdev */
    cdev_init(&devp->cdev, &globalmem_fops);
    devp->cdev.owner = THIS_MODULE;
    devno = devp->dev_num;
    err = cdev_add(&devp->cdev, devno, 1);
    if (err)
    {
        printk(KERN_ALERT "Error %d adding %s dev!\n", err, DEV_NAME);
    }
    else
    {
        printk(KERN_ALERT "Dev Num=%d.\n", devno);
    }

    /* create /sys/class/{$DEV_NAME} */
    devp->mem_class = class_create(THIS_MODULE, DEV_NAME);
    /* create /dev/{$DEV_NAME} */
    device_create(devp->mem_class, NULL, devno, NULL, DEV_NAME);
  
    return err;
}

static int __init globalmem_init(void)
{
    int   result = 0;
    dev_t devno  = 0;

    result = alloc_chrdev_region(&devno, 0, 1, DEV_NAME);
    CHECK_RETURN(result < 0, result);

    globalmem_devp = (struct globalmem_dev *)
        kmalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
    if (!globalmem_devp)
    {
        result = -ENOMEM;
        goto fail_malloc;
    }
    memset(globalmem_devp, 0, sizeof(struct globalmem_dev));

    globalmem_devp->dev_name = DEV_NAME;
    globalmem_devp->dev_num  = devno;
    result = globalmem_setup_cdev(globalmem_devp);
    if (result)
    {
        goto fail_setup;
    }
    
    // init_MUTEX(&(globalmem_devp->sem));
    sema_init(&(globalmem_devp->sem), 1);

    return result;

fail_setup:
    kfree(globalmem_devp);
fail_malloc:
    unregister_chrdev_region(devno, 1);

    return result;
}

static void __exit globalmem_exit(void)
{
    dev_t                 devno = 0;
    struct globalmem_dev *devp  = (struct globalmem_dev*)globalmem_devp;

    if (NULL == devp)
    {
        ALERT("Nothing to remove.\n");
    }
    devno = devp->dev_num;

    /* del /dev/{$DEV_NAME} */
    device_destroy(devp->mem_class, devno); 
    /* del /sys/class/{$DEV_NAME} */
    class_destroy(devp->mem_class);
    /* del cdev */
    cdev_del(&(devp->cdev));
 
    kfree(devp);
    unregister_chrdev_region(devno, 1);

    return;
}

module_init(globalmem_init);
module_exit(globalmem_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gavin Ding");
MODULE_DESCRIPTION("Globalmem Module");
MODULE_ALIAS("Hello LDD");

