#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// sudo apt-get update
// sudo apt-get install build-essential libi2c-dev 
// sudo apt-get install i2c-tools

// https://github.com/shenki/linux-i2c-example

// https://www.abelectronics.co.uk/kb/article/1092/i2c-part-3-i2c-tools-in-linux
// https://www.linkedin.com/pulse/understanding-i2c-communication-linux-beginners-guide-soheil-nazari

// https://www.circuitbasics.com/basics-of-the-i2c-communication-protocol/
// https://kotyara12.ru/iot/esp32_i2c/
// https://newhavendisplay.com/blog/i2c-communication-interface/
// http://kmpu.ru/other/bus_i2c/index.html

// https://stackoverflow.com/questions/63430327/how-can-i-emulate-an-i2c-device-on-linux
// https://eraretuya.github.io/2016/12/10/the-i2c-stub-in-action/
// https://hackaday.io/project/178271-i2c-master-mode-emulator

/*****************************************************************************************/
// sudo modprobe i2c-dev
// sudo modprobe i2c-stub chip_addr=0x03
// i2cdetect -l
// i2c-1	unknown   	SMBus stub driver               	N/A

// gcc -Wall -std=gnu11 -ggdb i2c_example.c
// sudo ./a.out
/*****************************************************************************************/

/*****************************************************************************************/
// sudo modprobe i2c-dev
// sudo modprobe i2c-stub chip_addr=0x1C
// sudo dmesg | tail
// i2c-stub: Virtual chip at 0x1c
// i2cdetect -l
// i2c-1	unknown   	SMBus stub driver               	N/A
// sudo i2cset 1 0x1C 0x07 0x04 b
// I will write to device file /dev/i2c-1, chip address 0x1c, data address 0x07, data 0x04, mode byte.
// sudo i2cset 1 0x1C 0x22 0x64 w
// I will write to device file /dev/i2c-1, chip address 0x1c, data address 0x22, data 0x64, mode word.
// ls /sys/class/i2c-adapter/i2c-1/
// delete_device  i2c-dev/       name           new_device     power/         subsystem/     uevent 
// sudo su
// echo al3320a 0x1c > /sys/class/i2c-adapter/i2c-1/new_device
// 1-001c  delete_device  i2c-dev  name  new_device  power  subsystem  uevent
// dmesg | tail
// i2c i2c-1: new_device: Instantiated device al3320a at 0x1c
// ls /sys/bus/iio/devices/iio:device0/
// in_illuminance_raw  in_illuminance_scale  in_illuminance_scale_available  name  power  subsystem  uevent
// cat /sys/bus/iio/devices/iio:device0/name
// al3320a
// cat /sys/bus/iio/devices/iio:device0/in_illuminance_scale_available
// 0.512 0.128 0.032 0.01
// cat /sys/bus/iio/devices/iio:device0/in_illuminance_scale
// 0.032000
// echo 0.512 > /sys/bus/iio/devices/iio:device0/in_illuminance_scale
// cat /sys/bus/iio/devices/iio:device0/in_illuminance_scale
// 0.512000
// cat /sys/bus/iio/devices/iio:device0/in_illuminance_raw
// 100
// modprobe -r i2c-stub
/*****************************************************************************************/

// ls /sys/bus/i2c/drivers/<TBD>
// echo <TBD>-<TBD> > /sys/bus/i2c/drivers/<TBD>/unbind
// (i2c-0)
//      (0)-(004a) bind uevent unbind

// i2cdetect	Detect I2C chips connected to the bus.
// i2cdump	Examine and read I2C registers on a connected device.
// i2cget	Read from I2C/SMBus chip registers on a connected device.
// i2cset	Set I2C registers on a connected device with new data or values.
// i2ctransfer	Send user-defined I2C messages in one transfer to a connected device.

static inline int32_t i2c_access(
	int file,
	char read_write,
	uint8_t command,
	int size,
	union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args = {};
	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;
	return ioctl(file, I2C_SMBUS, &args);
}


static inline int32_t i2c_read_byte(
	int file,
	uint8_t command)
{
	union i2c_smbus_data data = {};
	int32_t res = i2c_access(file, I2C_SMBUS_READ, command, I2C_SMBUS_BYTE_DATA, &data);
	if (res != 0) {
		err(errno, "Access failed: %d", res);
		return -1;
	}
	else {
		return (0x0FF & data.byte);
	}
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	uint8_t data = 0;
	const uint8_t addr = 0x3;
	const uint8_t reg = 0;
	const char *path = "/dev/i2c-1";
	int file = -1;
	int rc = 0;

	//const uint8_t addr = 0x76;
	//const uint8_t addr = 0x4a;
	//const uint8_t reg = 0x0d;
	//const char *path = "/dev/i2c-0";

	file = open(path, O_RDWR);
	if (file < 0) {
		err(errno, "Cannot open '%s'", path); 
	}

	rc = ioctl(file, I2C_SLAVE, addr);
	//rc = ioctl(file, I2C_SLAVE_FORCE, addr);
	if (rc < 0) {
		err(errno, "Cannot set device address '0x%02x'", addr);
	}

	data = i2c_read_byte(file, reg);
	printf("Device %s at address 0x%02x with command 0x%02x: 0x%02x\n",
		path, addr, reg, data);

	return 0;
}
