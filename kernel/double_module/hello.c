#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include  <linux/cdev.h>
#include <linux/device.h>



/*模块的许可证申明*/
MODULE_LICENSE("GPL");

dev_t  devno;

int  marjor  = 550;
int  minor   = 0;

struct  cdev  cdev;
struct  class  * cls;

static int hello_open (struct inode *inode, struct file *file)
{
	printk("minor  = %d\n",iminor(inode));
	printk("hello_open   \n");
	return  0;

}

static ssize_t hello_read (struct file *file, char __user *buf, size_t  size, loff_t *loff)
{

	printk("hello_read  \n");
	return  0;

}
static ssize_t hello_write (struct file *file, const char __user *buf, size_t  size, loff_t *loff)
{

	printk("hello_write  \n");
	return   0;

}
struct  file_operations  hello_fops  = {

	.open  =  hello_open,
	.read   = hello_read,
	.write  = hello_write,


};

/*用户自定义的加载函数*/
int  hello_init(void)
{

	int  ret;

	/*申请资源*/
	/*申请设备号*/
	devno  =  MKDEV(marjor,minor);
	/*注册设备号*/
	ret  = register_chrdev_region(devno,4,"hello");
	if(0  != ret)
	{

		printk("register_chrdev_region fail\n");
		return  -EBUSY;

	}
	/*初始化cdev结构体*/

	/*将cdev结构体添加系统中*/
	cdev_add(&cdev,devno,4);

	cdev_init(&cdev,&hello_fops);
	cls  = class_create(THIS_MODULE,"fsled");
	//注册次设备号
	device_create(cls,NULL,devno,NULL,"led0");
	device_create(cls,NULL,devno  +1,NULL,"led1");
	device_create(cls,NULL,devno  +2,NULL,"led2");
	device_create(cls,NULL,devno  +3,NULL,"led3");
	printk("hello_init  \n");//在驱动程序使用printk打印，在应用程序printf
	return  0;
}
/*用户自定义的卸载函数*/
void hello_exit(void)
{

	/*释放设备资源*/
	device_destroy(cls,devno+3);
	device_destroy(cls,devno+2);
	device_destroy(cls,devno+1);
	device_destroy(cls,devno+0);

	class_destroy(cls);

	/*删除cdev结构体*/
	cdev_del(&cdev);
	/*释放设备号*/
	unregister_chrdev_region(devno,4);

	printk("hello_exit\n");

}
/*加载函数*/
module_init(hello_init);
/*卸载函数*/
module_exit(hello_exit);

//module_platform_driver()
