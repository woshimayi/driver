/*
 * @brief : PWM子系统
 * @date :  2021-11-xx
 * @version : v1.0.0
 * @Change Logs:
 * @date
 * author
 *          notes:
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/io.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>

#if 0

#define RED_LED_DTS_COMPATIBLE "red_led,pwm" /* 设备树节点匹配属性 */

#define LED_PWM_CMD_SET_DUTY 0x01
#define LED_PWM_CMD_SET_PERIOD 0x02
#define LED_PWM_CMD_SET_BOTH 0x03
#define LED_PWM_CMD_ENABLE 0x04
#define LED_PWM_CMD_DISABLE 0x05

struct led_pwm_param
{
    int duty_ns;
    int period_ns;
};

struct red_led_dev
{
    dev_t dev_no;
    struct cdev chrdev;
    struct class *led_class;
    struct device_node *dev_node;
    struct pwm_device *red_led_pwm;
};

static struct led_pwm_param led_pwm;
static struct red_led_dev led_dev;

static int red_led_drv_open(struct inode *node, struct file *file)
{
    int ret = 0;

    pwm_set_polarity(led_dev.red_led_pwm, PWM_POLARITY_INVERSED);
    pwm_enable(led_dev.red_led_pwm);

    printk("red_led_pwm open\r\n");
    return ret;
}

static ssize_t red_led_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    int err;

    if (size != sizeof(led_pwm))
        return -EINVAL;

    err = copy_from_user(&led_pwm, buf, size);
    if (err > 0)
        return -EFAULT;

    pwm_config(led_dev.red_led_pwm, led_pwm.duty_ns, led_pwm.period_ns);

    return 1;
}

static long _drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    void __user *my_user_space = (void __user *)arg;

    switch (cmd)
    {
    case LED_PWM_CMD_SET_DUTY:
        ret = copy_from_user(&led_pwm.duty_ns, my_user_space, sizeof(led_pwm.duty_ns));
        if (ret > 0)
            return -EFAULT;
        pwm_config(led_dev.red_led_pwm, led_pwm.duty_ns, led_pwm.period_ns);
        break;
    case LED_PWM_CMD_SET_PERIOD:
        ret = copy_from_user(&led_pwm.period_ns, my_user_space, sizeof(led_pwm.period_ns));
        if (ret > 0)
            return -EFAULT;
        pwm_config(led_dev.red_led_pwm, led_pwm.duty_ns, led_pwm.period_ns);
        break;
    case LED_PWM_CMD_SET_BOTH:
        ret = copy_from_user(&led_pwm, my_user_space, sizeof(led_pwm));
        if (ret > 0)
            return -EFAULT;
        pwm_config(led_dev.red_led_pwm, led_pwm.duty_ns, led_pwm.period_ns);
        break;
    case LED_PWM_CMD_ENABLE:
        pwm_enable(led_dev.red_led_pwm);
        break;
    case LED_PWM_CMD_DISABLE:
        pwm_disable(led_dev.red_led_pwm);
        break;
    }
    return 0;
}

static int red_led_drv_release(struct inode *node, struct file *filp)
{
    int ret = 0;

    pwm_config(led_dev.red_led_pwm, 0, 5000);
    printk("led pwm dev close\r\n");
    //    pwm_disable(led_dev.red_led_pwm);
    return ret;
}

static struct file_operations red_led_drv = {
    .owner = THIS_MODULE,
    .open = red_led_drv_open,
    .write = red_led_drv_write,
    .unlocked_ioctl = _drv_ioctl,
    .release = red_led_drv_release,
};

/*设备树的匹配列表 */
static struct of_device_id dts_match_table[] = {
    {
        .compatible = RED_LED_DTS_COMPATIBLE,
    },
    {},
};

static int led_red_driver_probe(struct platform_device *pdev)
{
    int err;
    int ret;
    struct device *tdev;
    struct device_node *child;

    tdev = &pdev->dev;
    child = of_get_next_child(tdev->of_node, NULL); /* 获取设备树子节点 */
    if (!child)
    {
        printk("0\n");
        return -EINVAL;
    }

    led_dev.red_led_pwm = devm_of_pwm_get(tdev, child, NULL); /* 从子节点中获取PWM设备 最后一个参数为 NULL 时, 获取设备数第一个设别，为 pwm-names 名称时, 获取相关device */
    if (IS_ERR(led_dev.red_led_pwm))
    {
        printk(KERN_ERR "can't get red_led_pwm!!\n");
        return -EFAULT;
    }

    ret = alloc_chrdev_region(&led_dev.dev_no, 0, 1, "red_led_pwm1");
    if (ret < 0)
    {
        printk("1\n");
        pr_err("Error: failed to register mbochs_dev, err: %d\n", ret);
        return ret;
    }

    cdev_init(&led_dev.chrdev, &red_led_drv);

    cdev_add(&led_dev.chrdev, led_dev.dev_no, 1);

    led_dev.led_class = class_create(THIS_MODULE, "red_led_pwm2");
    err = PTR_ERR(led_dev.led_class);
    if (IS_ERR(led_dev.led_class))
    {
        printk("3\n");
        goto failed1;
    }

    tdev = device_create(led_dev.led_class, NULL, led_dev.dev_no, NULL, "red_led_pwm"); // red_led_pwm3  创建 dev 下的设备名称
    if (IS_ERR(tdev))
    {
        printk("2\n");
        ret = -EINVAL;
        goto failed2;
    }

    printk(KERN_INFO "%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    return 0;
failed2:
    device_destroy(led_dev.led_class, led_dev.dev_no);
    class_destroy(led_dev.led_class);
failed1:
    cdev_del(&led_dev.chrdev);
    unregister_chrdev_region(led_dev.dev_no, 1);
    return ret;
}

int led_red_driver_remove(struct platform_device *dev)
{
    // pwm_disable(led_dev.red_led_pwm);
    // pwm_free(led_dev.red_led_pwm);
    printk(KERN_INFO "driver remove %s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    device_destroy(led_dev.led_class, led_dev.dev_no);
    class_destroy(led_dev.led_class);
    unregister_chrdev_region(led_dev.dev_no, 1);
    cdev_del(&led_dev.chrdev);

    return 0;
}

static struct platform_driver red_led_platform_driver = {
    .probe = led_red_driver_probe,
    .remove = led_red_driver_remove,
    .driver = {
        .name = "red_led",
        .owner = THIS_MODULE,
        .of_match_table = dts_match_table, // 通过设备树匹配
    },
};

module_platform_driver(red_led_platform_driver);

MODULE_AUTHOR("XGJ");
MODULE_LICENSE("GPL");


#else
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/pwm.h>

struct pwm_rgb {
    struct pwm_device *red;
    struct pwm_device *green;
    struct pwm_device *blue;
};

static struct pwm_rgb rgb_led;

static int pwm_rgb_driver_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    struct pwm_args args;
    int ret;

    // Get the PWM channels from the device tree
    rgb_led.red = of_pwm_get(np, "red");
    if (!rgb_led.red) {
        dev_err(&pdev->dev, "failed to get red PWM\n");
        return -ENODEV;
    }

    rgb_led.green = of_pwm_get(np, "green");
    if (!rgb_led.green) {
        dev_err(&pdev->dev, "failed to get green PWM\n");
        ret = -ENODEV;
        goto err_put_red;
    }

    rgb_led.blue = of_pwm_get(np, "blue");
    if (!rgb_led.blue) {
        dev_err(&pdev->dev, "failed to get blue PWM\n");
        ret = -ENODEV;
        goto err_put_green;
    }

    // Set the PWM period and duty cycle for each color component
    args.period = 1000000;  // 1ms period

    // Red LED
    args.duty_cycle = 500000;  // 0.5ms duty cycle
    ret = pwm_config(rgb_led.red, &args);
    if (ret < 0) {
        dev_err(&pdev->dev, "failed to configure red PWM\n");
        goto err_put_blue;
    }

    // Green LED
    args.duty_cycle = 750000;  // 0.75ms duty cycle
    ret = pwm_config(rgb_led.green, &args);
    if (ret < 0) {
        dev_err(&pdev->dev, "failed to configure green PWM\n");
        goto err_put_blue;
    }

    // Blue LED
    args.duty_cycle = 250000;  // 0.25ms duty cycle
    ret = pwm_config(rgb_led.blue, &args);
    if (ret < 0) {
        dev_err(&pdev->dev, "failed to configure blue PWM\n");
        goto err_put_blue;
    }

    // Enable the PWM outputs
    ret = pwm_enable(rgb_led.red);
    if (ret < 0) {
        dev_err(&pdev->dev, "failed to enable red PWM\n");
        goto err_put_blue;
    }

    ret = pwm_enable(rgb_led.green);
    if (ret < 0) {
        dev_err(&pdev->dev, "failed to enable green PWM\n");
        goto err_put_blue;
    }

    ret = pwm_enable(rgb_led.blue);
    if (ret < 0) {
        dev_err(&pdev->dev, "failed to enable blue PWM\n");
        goto err_put_blue;
    }

    dev_info(&pdev->dev, "RGB LED initialized\n");

    return 0;

err_put_blue:
    pwm_put(rgb_led.blue);
err_put_green:
    pwm_put(rgb_led.green);
err_put_red:
    pwm_put(rgb_led.red);

    return ret;
}

static int pwm_rgb_driver_remove(struct platform_device *pdev)
{
    // Disable and free the PWM devices
    pwm_disable(rgb_led.red);
    pwm_disable(rgb_led.green);
    pwm_disable(rgb_led.blue);

    pwm_put(rgb_led.red);
    pwm_put(rgb_led.green);
    pwm_put(rgb_led.blue);

    dev_info(&pdev->dev, "RGB LED removed\n");

    return 0;
}

static const struct of_device_id pwm_rgb_driver_of_match[] = {
    { .compatible = "fsl,imx6ull-pwm-rgb", },
    {},
};
MODULE_DEVICE_TABLE(of, pwm_rgb_driver_of_match);

static struct platform_driver pwm_rgb_driver = {
    .probe = pwm_rgb_driver_probe,
    .remove = pwm_rgb_driver_remove,
    .driver = {
        .name = "imx6ull-pwm-rgb",
        .of_match_table = pwm_rgb_driver_of_match,
        .owner = THIS_MODULE,
    },
};

static int __init pwm_rgb_driver_init(void)
{
    return platform_driver_register(&pwm_rgb_driver);
}

static void __exit pwm_rgb_driver_exit(void)
{
    platform_driver_unregister(&pwm_rgb_driver);
}

module_init(pwm_rgb_driver_init);
module_exit(pwm_rgb_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("i.MX6ULL PWM RGB LED driver");

#endif