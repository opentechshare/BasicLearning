/*
* file : globalfifo.c 
* desc : a global fifo device for linux device driver development learning
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
#include <linux/wait.h>
#include <linux/poll.h>
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

#define DEV_NAME         "globalfifo"           /* device name */
#define GLOBALFIFO_SIZE  0x1000                 /* global fifo size, 4KB */

#define GLOBALFIFO_MAGIC 'G'
#define FIFO_CLEAR _IO(GLOBALFIFO_MAGIC, 0)     /* clear global fifo cmd word */

/* globalfifo device struct */
struct globalfifo_dev 
{
    dev_t                 dev_num;              /* device number in kernel */
    char                 *dev_name;             /* device name */
    struct cdev           cdev;                 /* cdev structure */
    struct class         *mem_class;            /* device class */
    unsigned char         mem[GLOBALFIFO_SIZE]; /* global memory */
    unsigned int          current_len;          /* valiad data length of the fifo */
    struct semaphore      sem;                  /* semaphore for concurrency control */
    wait_queue_head_t     r_wait;               /* wait queue head for block read */
    wait_queue_head_t     w_wait;               /* wait queue head for block write */
    struct fasync_struct *async_queue;          /* asynchronous struct pointer, for reading */
};

struct globalfifo_dev *globalfifo_devp;         /* device struct pointer */

static int globalfifo_fasync(int fd, struct file *filp, int mode)
{
    int                    ret  = OK;
    struct globalfifo_dev *devp = (struct globalfifo_dev*)filp->private_data;

    ret = fasync_helper(fd, filp, mode, &(devp->async_queue));
    
    return ret;    
}

int globalfifo_open(struct inode *inodp,
                    struct file *filp)
{
    struct globalfifo_dev *devp;
    
    devp = container_of(inodp->i_cdev, struct globalfifo_dev, cdev);
    filp->private_data = devp;

    return OK;
}

int globalfifo_release(struct inode *inodp,
                       struct file *filp)
{
    UNUSED(inodp);
    UNUSED(filp);

    (void)globalfifo_fasync(-1, filp, 0);
    
    return OK;
}

static ssize_t globalfifo_read(struct file *filp,
                               char __user *buf,
                               size_t count,
                               loff_t *ppos)
{
    ssize_t                ret  = 0;
    struct globalfifo_dev *devp = (struct globalfifo_dev*)filp->private_data;
    DECLARE_WAITQUEUE(wait, current); 

    CHECK_RETURN(NULL == devp, 0);

    down(&(devp->sem));
    add_wait_queue(&(devp->r_wait), &wait);

    while (0 == devp->current_len)
    {
        if (filp->f_flags & O_NONBLOCK)
        {
            ret = -EAGAIN;
            goto sem_up;
        }

        __set_current_state(TASK_INTERRUPTIBLE);
        up(&(devp->sem));

        schedule();
        if (signal_pending(current))
        {
            ret = -ERESTARTSYS;
            goto queue_rm;
        }

        down(&(devp->sem));
    }
    
    count = count > devp->current_len ? devp->current_len : count;

    if (copy_to_user(buf, devp->mem, count))
    {
        ret = -EFAULT;
        goto sem_up;
    }
    else
    {
        memcpy(devp->mem, devp->mem + count, devp->current_len - count);
        devp->current_len -= count;
        
        printk(KERN_INFO "read %d byte(s), current_len:%d\n", count, devp->current_len);
        
        wake_up_interruptible(&(devp->w_wait));
        
        ret = count;
    }

sem_up:
    up(&(devp->sem));
queue_rm:
    remove_wait_queue(&(devp->r_wait), &wait);
    set_current_state(TASK_RUNNING);
    
    return ret;    
}

static ssize_t globalfifo_write(struct file *filp,
                                const char __user *buf,
                                size_t count,
                                loff_t *ppos)
{
    ssize_t                ret  = 0;
    size_t                 size = 0;
    struct globalfifo_dev *devp = (struct globalfifo_dev*)filp->private_data;
    DECLARE_WAITQUEUE(wait, current);

    CHECK_RETURN(NULL == devp, 0);
    
    down(&(devp->sem));
    add_wait_queue(&(devp->w_wait), &wait);

    while (GLOBALFIFO_SIZE == devp->current_len)
    {
        if (filp->f_flags & O_NONBLOCK)
        {
            ret = -EAGAIN;
            goto sem_up;
        }

        __set_current_state(TASK_INTERRUPTIBLE);
        up(&(devp->sem));

        schedule();
        if (signal_pending(current))
        {
            ret = -ERESTARTSYS;
            goto queue_rm;
        }

        down(&(devp->sem));
    }
    
    size = GLOBALFIFO_SIZE - devp->current_len;
    count = count > size ? size : count;
    
    if (copy_from_user(devp->mem + devp->current_len, buf, count))
    {
        ret = -EFAULT;
        goto sem_up;
    }
    else
    {
        devp->current_len += count;
        
        printk(KERN_INFO "write %d byte(s), current_len:%d\n", count, devp->current_len);

        wake_up_interruptible(&(devp->r_wait));

        if (devp->async_queue)
        {
            kill_fasync(&(devp->async_queue), SIGIO, POLL_IN);
        }

        ret = count;
    }

sem_up:
    up(&(devp->sem));
queue_rm:
    remove_wait_queue(&(devp->w_wait), &wait);
    set_current_state(TASK_RUNNING);

    return ret;    
}

typedef enum tagSeekPosition
{
    seek_head = 0,
    seek_curr = 1,

    seek_butt   
}seekPositionE;

static loff_t globalfifo_llseek(struct file *filp,
                                loff_t offset,
                                int orig)
{
    loff_t ret = 0;

    switch (orig)
    {
        case seek_head:  /* from file head */
            if (offset < 0)
            {
                ret = -EINVAL;
                break;
            }

            if ((unsigned int)offset > GLOBALFIFO_SIZE)
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
            
            if ((unsigned int)(filp->f_pos + offset) > GLOBALFIFO_SIZE)
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

    return ret;
}

static long globalfifo_ioctl(struct file *filp,
                             unsigned int cmd,
                             unsigned long arg)
{
    struct globalfifo_dev *devp = (struct globalfifo_dev*)filp->private_data;

    CHECK_RETURN(NULL == devp, 0);
    
    switch (cmd)
    {
        case FIFO_CLEAR:
            if (down_interruptible(&(devp->sem)))
            {
                return -ERESTARTSYS;
            }
            memset(devp->mem, 0, GLOBALFIFO_SIZE);
            up(&(devp->sem));
            
            printk(KERN_INFO "%s is set to zero\n", DEV_NAME);
            break;

        default:
            return -EINVAL;  /* other unsupported commands */
    }

    return 0;
}

static unsigned int globalfifo_poll(struct file *filp,
                                    poll_table *wait)
{
    unsigned int           mask = 0;
    struct globalfifo_dev *devp = (struct globalfifo_dev*)filp->private_data;

    CHECK_RETURN(NULL == devp, 0);

    down(&(devp->sem));

    poll_wait(filp, &(devp->r_wait), wait);
    poll_wait(filp, &(devp->w_wait), wait);

    if (0 != devp->current_len)
    {
        mask |= POLLIN | POLLRDNORM;
    }

    if (GLOBALFIFO_SIZE != devp->current_len)
    {
        mask |= POLLOUT | POLLWRNORM;
    }

    up(&(devp->sem));

    return mask;
}

static const struct file_operations globalfifo_fops =
{
    .owner          = THIS_MODULE,
    .open           = globalfifo_open,
    .release        = globalfifo_release,
    .read           = globalfifo_read,
    .write          = globalfifo_write,
    .llseek         = globalfifo_llseek,
    .unlocked_ioctl = globalfifo_ioctl,
    .poll           = globalfifo_poll,
    .fasync         = globalfifo_fasync,
};

static int __init globalfifo_setup_cdev(struct globalfifo_dev *devp)
{
    int   err   = 0;
    dev_t devno = 0;

    CHECK_RETURN(NULL == devp, -EFAULT);
    
    /* init & add cdev */
    cdev_init(&devp->cdev, &globalfifo_fops);
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

static int __init gloabalfifo_init(void)
{
    int   result = 0;
    dev_t devno  = 0;

    result = alloc_chrdev_region(&devno, 0, 1, DEV_NAME);
    CHECK_RETURN(result < 0, result);

    globalfifo_devp = (struct globalfifo_dev *)
        kmalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);
    if (!globalfifo_devp)
    {
        result = -ENOMEM;
        goto fail_malloc;
    }
    memset(globalfifo_devp, 0, sizeof(struct globalfifo_dev));

    globalfifo_devp->dev_name = DEV_NAME;
    globalfifo_devp->dev_num  = devno;
    
    result = globalfifo_setup_cdev(globalfifo_devp);
    if (result)
    {
        goto fail_setup;
    }

    // init_MUTEX(&(globalfifo_devp->sem));
    sema_init(&(globalfifo_devp->sem), 1);
    init_waitqueue_head(&(globalfifo_devp->r_wait));
    init_waitqueue_head(&(globalfifo_devp->w_wait));

    return result;
    
fail_setup:
    kfree(globalfifo_devp);
fail_malloc:
    unregister_chrdev_region(devno, 1);

    return result;
}

static void __exit globalfifo_exit(void)
{
    dev_t                  devno = 0;
    struct globalfifo_dev *devp  = globalfifo_devp;

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

module_init(gloabalfifo_init);
module_exit(globalfifo_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gavin Ding");
MODULE_DESCRIPTION("Globalfifo Module");
MODULE_ALIAS("LDD IO");

