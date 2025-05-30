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

// gcc -Wall -std=gnu11 -ggdb i2c_example.c
// sudo ./a.out

// ls /sys/bus/i2c/drivers/<TBD>
// echo <TBD>-<TBD> > /sys/bus/i2c/drivers/<TBD>/unbind
// (i2c-0)
//      (0)-(004a) bind uevent unbind

// i2cdetect	Detect I2C chips connected to the bus.
// i2cdump	Examine and read I2C registers on a connected device.
// i2cget	Read from I2C/SMBus chip registers on a connected device.
// i2cset	Set I2C registers on a connected device with new data or values.
// i2ctransfer	Send user-defined I2C messages in one transfer to a connected device.

static inline int32_t i2c_smbus_access(
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


static inline int32_t i2c_smbus_read_byte_data(
	int file,
	uint8_t command)
{
	union i2c_smbus_data data = {};
	int32_t res = i2c_smbus_access(file, I2C_SMBUS_READ, command, I2C_SMBUS_BYTE_DATA, &data);
	//int32_t res = i2c_smbus_access(file, I2C_SMBUS_READ, command, I2C_SMBUS_BYTE_DATA, &data);
	if (res != 0) {
		//err(errno, "Access failed: %d", res);
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
	//const uint8_t addr = 0x76;
	const uint8_t addr = 0x4a;
	//const uint8_t reg = 0x0d;
	const uint8_t reg = 0;
	const char *path = "/dev/i2c-0";
	int file = -1;
	int rc = 0;

	file = open(path, O_RDWR);
	if (file < 0) {
		err(errno, "Cannot open '%s'", path); 
	}

	rc = ioctl(file, I2C_SLAVE, addr);
	//rc = ioctl(file, I2C_SLAVE_FORCE, addr);
	if (rc < 0) {
		err(errno, "Cannot set device address '0x%02x'", addr);
	}

	data = i2c_smbus_read_byte_data(file, reg);
	printf("%s: device 0x%02x at address 0x%02x: 0x%02x\n",
		path, addr, reg, data);

	return 0;
}
