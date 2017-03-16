
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stddef.h>
#include "spidriver.h"
#include "kuria_utils.h"




static const char *device = "/dev/spidev0.0";

static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 30000000;
static uint16_t delay;
static int spi_fd;


uint32_t spi_write_read(void* usr_ref, uint8_t* wdata, uint32_t wlength,
        uint8_t* rdata, uint32_t rlength) {

	int ret;
    //printf("SPI write read %d %d\n",wlength, rlength);
    
	struct spi_ioc_transfer tr[2] = {
		[0].tx_buf = (unsigned long)wdata,
		[0].rx_buf = (unsigned long)NULL,
		[0].len = wlength,
		[0].delay_usecs = delay,
		[0].speed_hz = speed,
		[0].bits_per_word = bits,
		[1].tx_buf = (unsigned long)NULL,
		[1].rx_buf = (unsigned long)rdata,
		[1].len = rlength,
		[1].delay_usecs = delay,
		[1].speed_hz = speed,
		[1].bits_per_word = bits,
	};

	ret = ioctl(spi_fd, SPI_IOC_MESSAGE(2), tr);
	if (ret < 1){
		pabort("can't send spi message");
    }
    //printf("Read %d \n", rdata[0]);
    return 0;
}
uint32_t spi_read(void* usr_ref, uint8_t* data, uint32_t length) {

	int ret;

    //printf("SPI read %d\n", length);
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)NULL,
		.rx_buf = (unsigned long)data,
		.len = length,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1){
		pabort("can't send spi message");
    }
    /*
	for (ret = 0; ret < length; ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", data[ret]);
	}
	puts("");*/

    return 0;
}

uint32_t spi_write(void* usr_ref, uint8_t* data, uint32_t length) {

	int ret;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)data,
		.rx_buf = (unsigned long)NULL,
		.len = length,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1){
		pabort("can't send spi message");
    }
/*	for (ret = 0; ret < length; ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", data[ret]);
	}
	puts("");*/ 
    return 0;
}

uint32_t spi_init(void) {

	int ret = 0;

	spi_fd = open(device, O_RDWR);
	if (spi_fd < 0) {
		pabort("can't open device");
    }
	/*
	 * spi mode
	 */
    mode =0;
	ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1) {
		pabort("can't set spi mode");
    }
	ret = ioctl(spi_fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1) {
		pabort("can't get spi mode");
    }
	/*
	 * bits per word
	 */
	ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1) {
		pabort("can't set bits per word");
    }
	ret = ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1) {
		pabort("can't get bits per word");
    }
	/*
	 * max speed hz
	 */
	ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		pabort("can't set max speed hz");
    }
	ret = ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		pabort("can't get max speed hz");
    }
	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%f MHz)\n", speed, speed/1000000.);

    return 0;
}

uint32_t spi_close(void) {
    close( spi_fd);

    return 0;
}
