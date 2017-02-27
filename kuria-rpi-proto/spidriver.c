

#include "spidriver.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "kuria_utils.h"

static const char *device = "/dev/spidev0.0";

static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 24000000;
static uint16_t delay;

uint32_t spi_write_read(void* usr_ref, uint8_t* wdata, uint32_t wlength,
        uint8_t* rdata, uint32_t rlength) {

	int ret;
    int fd = *(int*)usr_ref;
	
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)data,
		.rx_buf = (unsigned long)data,
		.len = length,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1){
		pabort("can't send spi message");
    }
	for (ret = 0; ret < length; ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", data[ret]);
	}
	puts("");
    return 0;
}
uint32_t spi_read(void* usr_ref, uint8_t* data, uint32_t length) {

	int ret;
    int fd = *(int*)usr_ref;
	
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)data,
		.rx_buf = (unsigned long)data,
		.len = length,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1){
		pabort("can't send spi message");
    }
	for (ret = 0; ret < length; ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", data[ret]);
	}
	puts("");
    return 0;
}

uint32_t spi_write(void* usr_ref, uint8_t* data, uint32_t length) {
    return 0;
}

uint32_t spi_init(int* spi_fd) {

	int ret = 0;

	*spi_fd = open(device, O_RDWR);
	if (*spi_fd < 0) {
		pabort("can't open device");
    }
	/*
	 * spi mode
	 */
	ret = ioctl(*spi_fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1) {
		pabort("can't set spi mode");
    }
	ret = ioctl(*spi_fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1) {
		pabort("can't get spi mode");
    }
	/*
	 * bits per word
	 */
	ret = ioctl(*spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1) {
		pabort("can't set bits per word");
    }
	ret = ioctl(*spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1) {
		pabort("can't get bits per word");
    }
	/*
	 * max speed hz
	 */
	ret = ioctl(*spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		pabort("can't set max speed hz");
    }
	ret = ioctl(*spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		pabort("can't get max speed hz");
    }
	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

    return 0;
}
