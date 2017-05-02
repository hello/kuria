
/**
 * @file
 * @brief Kuria SPI driver for PI protor.
 *
 */

#ifndef SPIDRIVER_H
#define SPIDRIVER_H

#include <stdint.h>

uint32_t spi_read(void* usr_ref, uint8_t* data, uint32_t length);
uint32_t spi_write(void* usr_ref, uint8_t* data, uint32_t length);
uint32_t spi_init(void);
uint32_t spi_write_read(void* usr_ref, uint8_t* wdata, uint32_t wlength,
        uint8_t* rdata, uint32_t rlength);
uint32_t spi_close(void);





#endif
