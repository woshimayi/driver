/* It is a simple testing */

//#include "/usr/include/libusb-1.0/libusb.h"  // libusb head file
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <libusb.h>


#define VID 0x16c0      // get of lsusb
#define PID 0x05df      // get of lsusb

struct libusb_device_handle *devh = NULL;

//unsigned char openstr[] = {0xa1, 0x01, 0x00, 0x03, 0x00, 0x00, 0x08, 0x00};

int main()
{
	/* usb init before libusb_open* */
	int ret = libusb_init(NULL);
	if (ret < 0)
	{
		perror("libusb_init");
		return ret;
	}
	/* end */

	/* open device with vid and pid, must after libusb_init */
	devh = libusb_open_device_with_vid_pid(NULL, VID, PID);
	if (!devh)
	{
		perror("libusb_open_device_with_pid_vid");
		libusb_exit(NULL);
	}
	/* end */

	/* claim interface */
	ret = libusb_claim_interface(devh, 0);
	if (ret < 0)
	{
		perror("libusb_claim_interface");
		devh = NULL;
		libusb_close(devh);
		return ret;
	}
	/* end */

	/* open device data */
	unsigned char open_data[8];
	memset(open_data, 0, sizeof(open_data));
	if (0 > libusb_control_transfer(devh, 0xa1, 0x01, 0x3000, 0x00, open_data, 0x08, 1000))
	{
		perror("libusb_control_transfer");
	}
	printf("receive data: %s\n", open_data);
	int i = 0;
	for (i = 0; i < 8; i++)
	{
		printf("%02x\t", open_data[i]);
	}
	putchar(10);
	/* end */

	/* lock relay */
	unsigned char lock_data[] = {0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	if (0 > libusb_control_transfer(devh, 0x21, 0x09, 0x0000, 0x00, lock_data, 0x08, 1000))
	{
		perror("libusb_control_transfer");
	}
	/* end */

	/* delay */
	sleep(2);

	/* unlock relay */
	unsigned char unlock_data[] = {0xfd, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	if (0 > libusb_control_transfer(devh, 0x21, 0x09, 0x3000, 0x00, unlock_data, 0x08, 1000))
	{
		perror("libusb_control_transfer");
	}
	/* end */

	/* release claim interface */
	libusb_release_interface(devh, 0);
	/* end */

	/* close device */
	libusb_close(devh);

	return 0;
}

