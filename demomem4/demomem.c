#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#define BUF_SIZE 1024
#define MAJOR_NUM 168
#define MINOR_NUM 0

struct demo_device
{
    char *buffer; //将其改为指针
    struct cdev cdev;  
    struct class *cls;
    struct device *device;  
};
static struct demo_device *demo_dev;

static int demo_open(struct inode *node, struct file *file)
{
    printk(KERN_INFO "Enter: %s\n",__func__);
    file->private_data = (void *)demo_dev;
    return 0;
}  

static int demo_release(struct inode *node, struct file *file)
{
    printk(KERN_INFO "Enter: %s\n",__func__);
    return 0;
}

static ssize_t demo_read(struct file *file, char __user *buf, size_t size, loff_t *pos)//将其中内容重新填充
{
    int ret;
    int read_bytes;
    struct demo_device *demo = file->private_data;
    char *kbuf = demo->buffer + *pos;//将指定buffer偏移后给kbuff

    if(*pos >= BUF_SIZE)//判断是否读到结尾
        return 0;
    
    if(size > (BUF_SIZE - *pos))//判断的buffer和要读的buffer哪个大
        read_bytes  = BUF_SIZE - *pos;//如果剩下的buffer小，那只能读取剩下的buffer
    else
        read_bytes = size;
    
    ret = raw_copy_to_user(buf,kbuf,read_bytes);//buf用户空间的地址，kbuf内核空间的地址
    if(ret != 0)
        return -EFAULT;
   *pos += read_bytes;
   return read_bytes;
    
}

static ssize_t demo_write(struct file *file, const char __user *buf, size_t size, loff_t *pos)//新建write函数
{
    int ret;
    int write_bytes;
    struct demo_device *demo = file->private_data;
    char *kbuf = demo->buffer + *pos;

    if(*pos >= BUF_SIZE)//判断是否读到结尾
        return -1;
    
    if(size > (BUF_SIZE - *pos))//判断的buffer和要读的buffer哪个大
        write_bytes  = BUF_SIZE - *pos;//如果剩下的buffer小，那只能读取剩下的buffer
    else
        write_bytes = size;

    ret = raw_copy_from_user(kbuf,buf,write_bytes);//buf用户空间的地址，kbuf内核空间的地址
    if(ret != 0)
        return -EFAULT;
   *pos += write_bytes;
   return write_bytes;
}

static struct file_operations demo_operation = {
    
    .open = demo_open,
    .release = demo_release,
    .read = demo_read,
    .write = demo_write,//添加demo_write
};

static int __init demo_init(void)
{
    int ret = -1;
    dev_t dev_no;

    demo_dev = (struct demo_device *)kmalloc(sizeof(struct demo_device),GFP_KERNEL);
    if(!demo_dev)
    {
        printk(KERN_ERR "failed to malloc demo devices\n");
        ret = -ENOMEM;
        goto ERROR_MALLOC_DEVICE;
    }

    demo_dev->buffer = (char *)kmalloc(BUF_SIZE,GFP_KERNEL);//动态分配内存空间
    if(!demo_dev->buffer)//判断内存空间是否分配成功
    {   printk(KERN_ERR "mall %d bytes failed\n",BUF_SIZE);
        ret  = -ENOMEM;
        goto ERROR_MALLOC_BUFFER;
    }
    dev_no = MKDEV(MAJOR_NUM,MINOR_NUM);
    printk(KERN_INFO "Enter: %s\n",__func__);
    //init for demo char
    cdev_init(&demo_dev->cdev, &demo_operation);
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

    ret = cdev_add(&demo_dev->cdev,dev_no,1);//注册一个主设备，默认有一个次设备，所以次设为1。
    if(ret < 0)
    {
        printk(KERN_ERR "cdev add failed\n");
        
        goto ERROR_CDEV_ADD;
    }
    //create a demomem class in /sys/class/demo
    demo_dev->cls = class_create(THIS_MODULE,"demo");
    if(IS_ERR(demo_dev->cls))
    {
        ret = PTR_ERR(demo_dev->cls);
        goto ERROR_CLASS_CREATE;
    }
    //create ademomem device in /sys/class/demo/demomem
    demo_dev->device = device_create(demo_dev->cls,NULL,dev_no,NULL,"demomem");
    if(IS_ERR(demo_dev->device))
    {
        ret = PTR_ERR(demo_dev->device);
        goto ERROR_DEVICE_CREATE;
    }
    printk(KERN_INFO "dome init done\n");

    return 0;
ERROR_DEVICE_CREATE:
    class_destroy(demo_dev->cls);
ERROR_CLASS_CREATE:
    cdev_del(&demo_dev->cdev);
ERROR_CDEV_ADD:
    unregister_chrdev_region(dev_no,1);
ERROR_CHRDEV_REGION:
    kfree(demo_dev->buffer);//从这开始后面无论在那步骤失败都要释放demo_dev.buffer
    demo_dev->buffer = NULL;
ERROR_MALLOC_BUFFER:
    kfree(demo_dev);
    demo_dev = NULL;
ERROR_MALLOC_DEVICE:
 return 0;

}

static void __exit demo_exit(void)
{
    
    printk(KERN_INFO "Enter: %s\n",__func__);
    cdev_del(&demo_dev->cdev);
    unregister_chrdev_region(MKDEV(MAJOR_NUM,MINOR_NUM),1);

    kfree(demo_dev);
    demo_dev = NULL;
    kfree(demo_dev->buffer);//卸载时将内存释放，防止内存泄露
    demo_dev->buffer = NULL;//释放完空间后将指针置为空，防止误操作
    printk(KERN_INFO "demo exit done\n");

}

module_init(demo_init);
module_exit(demo_exit);

MODULE_AUTHOR("zhangshaokang");
MODULE_LICENSE("GPL");
