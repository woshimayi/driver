#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/mutex.h>


// Define these values to match your devices
//joystick: VENDOR_ID 0x0E8F, PRODUCT_ID 0X0002
//icbc: VENDOR_ID 0x096E, PRODUCT_ID 0X0010
#define USB_JOYSTICK_VENDOR_ID  0x10c4
#define USB_JOYSTICK_PRODUCT_ID 0x0000


/* table of devices that work with this driver */
static struct usb_device_id joystick_table[] =
{
    { USB_DEVICE(USB_JOYSTICK_VENDOR_ID, USB_JOYSTICK_PRODUCT_ID) },
    { }                 /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, joystick_table);


//masks for each key, see file KEY_MASK for more details
#define KEYMASK_UP      0x00
#define KEYMASK_DOWN    0xFF
#define KEYMASK_LEFT    0x00
#define KEYMASK_RIGHT   0xFF
#define KEYMASK_L1      0x01
#define KEYMASK_L2      0x04
#define KEYMASK_R1      0x02
#define KEYMASK_R2      0x08
#define KEYMASK_Y       0x1F
#define KEYMASK_A       0x4F
#define KEYMASK_X       0x8F
#define KEYMASK_B       0x2F
#define KEYMASK_SELECT  0x10
#define KEYMASK_START   0x20
#define INTERV_MILLI(ms)    (ms*HZ/1000)

static struct fasync_struct *fasync_queue;
static char *gb_data;

// Get a minor range for your devices from the usb maintainers
#define USB_JOYSTICK_MINOR_BASE 192
// Structure to hold all of our device specific stuff

struct usb_joystick
{
    struct usb_device       *udev;          /* the usb device for this device */
    struct usb_interface    *interface;     /* the interface for this device */
    struct semaphore        limit_sem;      /* limiting the number of writes in progress */
    struct urb              *in_urb;        /* the urb to read data with */
    unsigned char           *in_buffer; /* the buffer to receive data */
    size_t                  in_buffer_size;     /* the size of the receive buffer */
    char                    *data;
    dma_addr_t              data_dma;
    __u8                    in_endpointAddr;    /* the address of the bulk in endpoint */
    __u8                    out_endpointAddr;   /* the address of the bulk out endpoint */
    struct kref             kref;
};

static int joystick_fasync(int fd, struct file *file, int on);

static int joystick_open(struct inode *inode, struct file *file)
{
    printk("joystick open.\n");
    return 0;
}

static int joystick_release(struct inode *inode, struct file *file)
{
    printk("joystick release.\n");
    joystick_fasync(-1, file, 0);
    return 0;
}

static ssize_t joystick_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    printk("joystick read.\n");
    copy_to_user(buf, gb_data, 8);
    return 0;
}

static ssize_t joystick_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    printk("joystick write.\n");
    return 0;
}

static int joystick_fasync(int fd, struct file *file, int on)
{
    int retval;
    printk("joystick fasynchronize.\n");
    retval = fasync_helper(fd, file, on, &fasync_queue);
    return retval;
}

static const struct file_operations joystick_fops =
{
    .owner      =       THIS_MODULE,
    .open       =       joystick_open,
    .release    =       joystick_release,
    .read       =       joystick_read,
    .write      =       joystick_write,
    .fasync     =       joystick_fasync,
};

static void joystick_irq(struct urb *urb)
{
    struct usb_joystick *js_dev = urb->context;
    char *data;
    static unsigned long jiff_UP = 0;
    static unsigned long jiff_DOWN = 0;
    static unsigned long jiff_LEFT = 0;
    static unsigned long jiff_RIGHT = 0;
    static unsigned long jiff_Y = 0;
    static unsigned long jiff_A = 0;
    static unsigned long jiff_X = 0;
    static unsigned long jiff_B = 0;
    static unsigned long jiff_L1 = 0;
    static unsigned long jiff_L2 = 0;
    static unsigned long jiff_R1 = 0;
    static unsigned long jiff_R2 = 0;
    static unsigned long jiff_SELECT = 0;
    static unsigned long jiff_START = 0;
    static int cnt = 0;
    int retval, status;
    status = urb->status;
    cnt++;
    if (cnt == 1000)
    {
        cnt = 0;
        printk("urb->status=%d\n", status);
    }
    switch (status)
    {
        case 0:
            printk("success,get data...\n");
            data = js_dev->data;
            if ((data[0] != (char)0x7F)
                    || (data[1] != (char)0x7F)
                    || (data[2] != (char)0x7F)
                    || (data[3] != (char)0x7F)
                    || (data[4] != (char)0xFF)
                    || (data[5] != (char)0x0F)
                    || (data[6] != (char)0x00)
                    || (data[7] != (char)0x00))
            {

                printk("%02X %02X %02X %02X %02X %02X %02X %02X\n",
                       (char)data[0], (char)data[1], (char)data[2], (char)data[3],
                       (char)data[4], (char)data[5], (char)data[6], (char)data[7]);

                //direction keys
                switch (data[2])
                {
                    case KEYMASK_LEFT:
                        if (jiffies > (jiff_LEFT + INTERV_MILLI(200)))
                        {
                            jiff_LEFT = jiffies;
                            printk("LEFT pressed.\n");
                            goto signal_app;
                        }
                        break;
                    case KEYMASK_RIGHT:
                        if (jiffies > (jiff_RIGHT + INTERV_MILLI(200)))
                        {
                            jiff_RIGHT = jiffies;
                            printk("RIGHT pressed.\n");
                            goto signal_app;
                        }
                        break;
                }
                switch (data[3])
                {
                    case KEYMASK_UP:
                        if (jiffies > (jiff_UP + INTERV_MILLI(200)))
                        {
                            jiff_UP = jiffies;
                            printk("UP pressed.\n");
                            goto signal_app;
                        }
                        break;
                    case KEYMASK_DOWN:
                        if (jiffies > (jiff_DOWN + INTERV_MILLI(200)))
                        {
                            jiff_DOWN = jiffies;
                            printk("DOWN pressed.\n");
                            goto signal_app;
                        }
                        break;
                }
                //general function keys
                switch (data[5])
                {
                    case KEYMASK_Y:
                        if (jiffies > (jiff_Y + INTERV_MILLI(200)))
                        {
                            jiff_Y = jiffies;
                            printk("Y pressed.\n");
                            goto signal_app;
                        }
                        break;
                    case KEYMASK_A:
                        if (jiffies > (jiff_A + INTERV_MILLI(200)))
                        {
                            jiff_A = jiffies;
                            printk("A pressed.\n");
                            goto signal_app;
                        }
                        break;
                    case KEYMASK_X:
                        if (jiffies > (jiff_X + INTERV_MILLI(200)))
                        {
                            jiff_X = jiffies;
                            printk("X pressed.\n");
                            goto signal_app;
                        }
                        break;
                    case KEYMASK_B:
                        if (jiffies > (jiff_B + INTERV_MILLI(200)))
                        {
                            jiff_B = jiffies;
                            printk("B pressed.\n");
                            goto signal_app;
                        }
                        break;
                }
                //advanced function keys
                switch (data[6])
                {
                    case KEYMASK_L1:
                        if (jiffies > (jiff_L1 + INTERV_MILLI(200)))
                        {
                            jiff_L1 = jiffies;
                            printk("L1 pressed.\n");
                            goto signal_app;
                        }
                        break;
                    case KEYMASK_L2:
                        if (jiffies > (jiff_L2 + INTERV_MILLI(200)))
                        {
                            jiff_L2 = jiffies;
                            printk("L2 pressed.\n");
                            goto signal_app;
                        }
                        break;
                    case KEYMASK_R1:
                        if (jiffies > (jiff_R1 + INTERV_MILLI(200)))
                        {
                            jiff_R1 = jiffies;
                            printk("R1 pressed.\n");
                            goto signal_app;
                        }
                        break;
                    case KEYMASK_R2:
                        if (jiffies > (jiff_R2 + INTERV_MILLI(200)))
                        {
                            jiff_R2 = jiffies;
                            printk("R2 pressed.\n");
                            goto signal_app;
                        }
                        break;
                    case KEYMASK_SELECT:
                        if (jiffies > (jiff_SELECT + INTERV_MILLI(200)))
                        {
                            jiff_SELECT = jiffies;
                            printk("SELECT pressed.\n");
                            goto signal_app;
                        }
                        break;
                    case KEYMASK_START:
                        if (jiffies > (jiff_START + INTERV_MILLI(200)))
                        {
                            jiff_START = jiffies;
                            printk("START pressed.\n");
                            goto signal_app;
                        }
                        break;
                }
            }
            break;
        default:
            break;
    }
    goto submit_urb;

signal_app:
    //notify our application that at least one key was pressed
    if (fasync_queue)
    {
        kill_fasync(&fasync_queue, SIGIO, POLL_IN);
        printk("notify our application that at least one key was pressed.\n");
    }

submit_urb:
    retval = usb_submit_urb(urb, GFP_ATOMIC);
    if (retval)
        printk("usb_submit_urb error.\n");
}


/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
static struct usb_class_driver joystick_class =
{
    .name =     "joystick%d",
    .fops =     &joystick_fops,
    .minor_base =   USB_JOYSTICK_MINOR_BASE,
};


static int joystick_probe(struct usb_interface *interface,
                          const struct usb_device_id *id)
{
    struct usb_device *dev = interface_to_usbdev(interface);
    struct usb_joystick *js_dev = NULL;
    struct usb_host_interface *iface_desc;
    struct usb_endpoint_descriptor *endpoint;
    size_t maxp;
    int pipe;
    int retval = -ENOMEM;
    printk("*************** Probe for joystick. ****************\n");
    //dev->udev = usb_get_dev(interface_to_usbdev(interface));
    //dev->interface = interface;
    iface_desc = interface->cur_altsetting;
    printk("numEndpoints=%d\n", iface_desc->desc.bNumEndpoints);

    if (iface_desc->desc.bNumEndpoints < 1)
    {
        printk("Finding interface...");
        return -ENODEV;
    }

    endpoint = &iface_desc->endpoint[0].desc;
    if (!endpoint)
    {
        printk("No endpoint");
        return -ENODEV;
    }
    printk("direction(%02X): ", endpoint->bEndpointAddress);

    if (endpoint->bEndpointAddress & USB_DIR_IN)
        printk("to host\n");
    else
        printk("to device\n");
    printk("Endpoint type: ");

    switch (endpoint->bmAttributes)
    {
        case 0:
            printk("control\n");
            break;
        case 1:
            printk("ISOC\n");
            break;
        case 2:
            printk("bulk\n");
            break;
        case 3:
            printk("intterupt\n");
            break;
        default:
            printk("Unkown.\n");
    }
    printk("MaxPacketSize: %d\n", endpoint->wMaxPacketSize);
    //obtain usb_device pointor from usb_interface
    dev = interface_to_usbdev(interface);
    pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
    maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));
    printk("pipe: %08X, maxp: %08X\n", pipe, maxp);
    printk("sizeof(struct usb_joystick)=%d\n", sizeof(struct usb_joystick));
    js_dev = kzalloc(sizeof(struct usb_joystick), GFP_KERNEL);
    printk("Oh Yeah!\n");
    if (!js_dev)
        goto fail0;
    kref_init(&js_dev->kref);
    js_dev->data = usb_alloc_coherent(dev, 64, GFP_ATOMIC, &js_dev->data_dma);
    if (!js_dev->data)
        goto fail1;
    gb_data = js_dev->data;
    js_dev->in_urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!js_dev->in_urb)
        goto fail2;
    usb_fill_int_urb(js_dev->in_urb, dev, pipe, js_dev->data, (maxp > 8 ? 8 : maxp), joystick_irq, js_dev,
                     endpoint->bInterval);
    js_dev->in_urb->transfer_dma = js_dev->data_dma;
    js_dev->in_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
    js_dev->udev = usb_get_dev(dev);
    js_dev->interface = interface;
    usb_set_intfdata(interface, js_dev);
    retval = usb_register_dev(interface, &joystick_class);
    if (retval)
    {
        //something prevented us from registering this driver
        err("Not able to get a minor for this driver.");
        usb_set_intfdata(interface, NULL);
        goto fail3;
    }
    usb_submit_urb(js_dev->in_urb, GFP_ATOMIC);
    dev_info(&interface->dev, "USB JoyStick now attaches to minor %d\n", interface->minor);
    printk("*************** End of probe ****************\n");
    return 0;
fail3:
    printk("fail3.\n");
    usb_free_urb(js_dev->in_urb);
fail2:
    printk("fail2.\n");
    usb_free_coherent(dev, 8, js_dev->data, js_dev->data_dma);
fail1:
    printk("fail1.\n");
    kfree(js_dev);
fail0:
    printk("fail0.\n");
    return -ENODEV;
}


static void joystick_disconnect(struct usb_interface *interface)
{
    struct usb_joystick *js_dev;
    int minor = interface->minor;
    js_dev = usb_get_intfdata(interface);

    usb_set_intfdata(interface, NULL);
    if (js_dev)
    {
        usb_kill_urb(js_dev->in_urb);
        //give back out minor
        usb_deregister_dev(interface, &joystick_class);
    }
    dev_info(&interface->dev, "USB minor#%d now disconnected", minor);
}

static struct usb_driver joystick_driver =
{
    .name =         "joystick",
    .probe =        joystick_probe,
    .disconnect =   joystick_disconnect,
    .id_table =     joystick_table,
};


//init USB skeleton module
static int __init usb_joystick_init(void)
{
    int result;
    /* register this driver with the USB subsystem */
    result = usb_register(&joystick_driver);
    if (result)
        printk("usb_register failed. Error number %d", result);
    printk("************** joystick driver initialized. ****************\n");
    return result;
}


//unload USB skeleton module
static void __exit usb_joystick_exit(void)
{
    /* deregister this driver with the USB subsystem */
    usb_deregister(&joystick_driver);
    printk("**************** joystick driver unloaded. ****************\n");
}


module_init(usb_joystick_init);
module_exit(usb_joystick_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cricket Long");

