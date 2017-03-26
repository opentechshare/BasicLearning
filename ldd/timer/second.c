/******************************************************************************

                  版权所有 (C), 2013-2023, 天马行空有限公司

 ******************************************************************************
  文 件 名   : second.c
  版 本 号   : 初稿
  作    者   : gavin
  生成日期   : 2013年2月24日
  最近修改   :
  功能描述   : 秒字符设备
  函数列表   :
  修改历史   :
  1.日    期   : 2013年2月24日
    作    者   : gavin
    修改内容   : 创建文件

******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>

#define SECOND_DEV "second"
#define UNUSED(x)  ((x) = (x))

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

/* second device struct */
typedef struct tag_second_dev
{
    dev_t              dev_num;
    char              *dev_name;
    struct cdev        cdev;       /* cdev struct */
    struct class      *dev_class;  /* device class */
    atomic_t           counter;    /* the seconds used totally */
    struct timer_list  s_timer;    /* timer for the dev */
}second_dev;
second_dev *second_devp;

second_dev* get_second_dev(void)
{
    return second_devp;
}

static void second_timer_handle(unsigned long arg)
{
    second_dev *devp = get_second_dev();

    UNUSED(arg);
    
    mod_timer(&(devp->s_timer), jiffies + HZ);
    atomic_inc(&(devp->counter));

    printk(KERN_NOTICE "current jiffies is %ld\n", jiffies);

    return;
}

static int second_open(struct inode *inodep, struct file *filp)
{
    second_dev *devp = get_second_dev();
    
    init_timer(&(devp->s_timer));
    devp->s_timer.function = &second_timer_handle;
    devp->s_timer.expires  = jiffies + HZ;

    add_timer(&(devp->s_timer));

    atomic_set(&(devp->counter), 0);
    
    return 0;
}

static int second_release(struct inode *inodep, struct file *filp)
{
    second_dev *devp = get_second_dev();
    
    del_timer(&(devp->s_timer));
    
    return 0;
}

static ssize_t second_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    second_dev *devp    = get_second_dev();
    int         counter = 0;

    counter = atomic_read(&(devp->counter));
    if (put_user(counter, (int*)buf))
    {
        return -EFAULT;
    }

    return sizeof(unsigned int);    
}

static const struct file_operations second_fops = 
{
    .owner   = THIS_MODULE,
    .open    = second_open,
    .release = second_release,
    .read    = second_read,
};

static int __init second_setup_cdev(second_dev *devp, int index)
{
    int   err   = 0;
    dev_t devno = 0;

    CHECK_RETURN(NULL == devp, -EFAULT);

    cdev_init(&(devp->cdev), &second_fops);
    devp->cdev.owner = THIS_MODULE;
    devno = devp->dev_num;
    
    err = cdev_add(&(devp->cdev), devno, 1);
    if (err)
    {
        printk(KERN_NOTICE "Error %d adding second dev %d!\n", err, index);
    }
    else
    {
        printk(KERN_NOTICE "Dev Num=%d, index=%d.\n", devno, index);
    }

    /* create /sys/class/second */
    devp->dev_class = class_create(THIS_MODULE, SECOND_DEV);
    device_create(devp->dev_class, NULL, devno, NULL, SECOND_DEV);
            
    return err;
}

static int __init second_init(void)
{
    int   result = 0;
    dev_t devno  = 0;

    result = alloc_chrdev_region(&devno, 0, 1, SECOND_DEV);
    CHECK_RETURN(result < 0, result);    

    second_devp = (second_dev *)kmalloc(sizeof(second_dev), GFP_KERNEL);
    if (!second_devp)
    {
        result = -ENOMEM;
        goto fail_malloc;
    }
    memset(second_devp, 0, sizeof(second_dev));
    second_devp->dev_num  = devno;
    second_devp->dev_name = SECOND_DEV;

    result = second_setup_cdev(second_devp, 0);
    if (result)
    {
        goto fail_setup;
    }
    
    return result;
    
fail_setup:
    kfree(second_devp);
fail_malloc:
    unregister_chrdev_region(devno, 1);
    
    return result;
}

static void __exit second_exit(void)
{
    dev_t       devno = 0;
    second_dev *devp  = get_second_dev();

    if (NULL == devp)
    {
        ALERT("Nothing to remove.\n");
    }
    devno = devp->dev_num;

    device_destroy(devp->dev_class, devno);
    class_destroy(devp->dev_class);

    cdev_del(&(devp->cdev));
    kfree(devp);

    unregister_chrdev_region(devno, 1);
    
    return;
}

module_init(second_init);
module_exit(second_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Gavin Ding");
MODULE_DESCRIPTION("Second Module");
MODULE_ALIAS("Ldd timer");

