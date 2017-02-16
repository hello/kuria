
/**
 * @file
 * @brief Kuria SPI driver for PI protor.
 *
 */

#ifndef SPIDRIVER_H
#define SPIDRIVER_H

uint32_t spi_read(void* usr_ref, uint8_t data, uint32_t length);
uint32_t spi_write(void* usr_ref, uint8_t data, uint32_t length);
uint32_t spi_init(void);

#endif
