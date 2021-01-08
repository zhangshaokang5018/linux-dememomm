#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>

#define BUF_SIZE 1024
#define MAJOR_NUM 168
#define MINOR_NUM 0
struct demo_device
{
    char buffer[BUF_SIZE];
    struct cdev cdev;  
    struct class *cls;
    struct device *device;  
};
static struct demo_device demo_dev;

static int demo_open(struct inode *node, struct file *file)
{
    printk(KERN_INFO "Enter: %s\n",__func__);
    return 0;
}  

static int demo_release(struct inode *node, struct file *file)
{
    printk(KERN_INFO "Enter: %s\n",__func__);
    return 0;
}

static ssize_t demo_read(struct file *file, char __user *buf, size_t size, loff_t *pos)
{
    printk(KERN_INFO "Enter: %s\n",__func__);
    return 0;
}

static struct file_operations demo_operation = {
    
    .open = demo_open,
    .release = demo_release,
    .read = demo_read,
    //.write
};

static int __init demo_init(void)
{
    int ret = -1;
    dev_t dev_no;
    dev_no = MKDEV(MAJOR_NUM,MINOR_NUM);
    printk(KERN_INFO "Enter: %s\n",__func__);
    //init for demo char
    cdev_init(&demo_dev.cdev, &demo_operation);
    //register device number
    ret = register_chrdev_region(dev_no,1,"demomem");//静态注册注册设备号时名字为demomem
    if(ret < 0)
    {
        ret = alloc_chrdev_region(&dev_no,0,1,"demomem");//在静态注册失败时，动态注册。0代表子设备号，1代表数量
        if(ret < 0)
        {
            printk(KERN_ERR "failed to register device number\n");
            goto ERROR_CHRDEV_REGION;
        }
       
    }

    ret = cdev_add(&demo_dev.cdev,dev_no,1);//注册一个主设备，默认有一个次设备，所以次设为1。
    if(ret < 0)
    {
        printk(KERN_ERR "cdev add failed\n");
        
        goto ERROR_CDEV_ADD;
    }
    //create a demomem class in /sys/class/demo
    demo_dev.cls = class_create(THIS_MODULE,"demo");
    if(IS_ERR(demo_dev.cls))
    {
        ret = PTR_ERR(demo_dev.cls);
        goto ERROR_CLASS_CREATE;
    }
    //create ademomem device in /sys/class/demo/demomem
    demo_dev.device = device_create(demo_dev.cls,NULL,dev_no,NULL,"demomem");
    if(IS_ERR(demo_dev.device))
    {
        ret = PTR_ERR(demo_dev.device);
        goto ERROR_DEVICE_CREATE;
    }
    printk(KERN_INFO "dome init done\n");

    return 0;
ERROR_DEVICE_CREATE:
    class_destroy(demo_dev.cls);
ERROR_CLASS_CREATE:
    cdev_del(&demo_dev.cdev);
ERROR_CDEV_ADD:
    unregister_chrdev_region(dev_no,1);
ERROR_CHRDEV_REGION:
    return 0;
}

static void __exit demo_exit(void)
{
    
    printk(KERN_INFO "Enter: %s\n",__func__);
    cdev_del(&demo_dev.cdev);
    unregister_chrdev_region(MKDEV(MAJOR_NUM,MINOR_NUM),1);

}

module_init(demo_init);
module_exit(demo_exit);

MODULE_AUTHOR("zhangshaokang");
MODULE_LICENSE("GPL");
