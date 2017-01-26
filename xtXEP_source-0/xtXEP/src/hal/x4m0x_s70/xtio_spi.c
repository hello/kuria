#include <stdint.h>
#include <string.h>
//#include "sysclk.h"
#include "ioport.h"
#include "ioport_pio.h"
#include "xep_hal.h"
#include "sams70q20.h"
#include "qspi.h"
#include "qspi_private.h"


#define SPI_CLOCK_POLARITY          0			// Clock polarity.
#define SPI_CLOCK_PHASE             0			// Clock phase.
#define SPI_DELAY_BEFORE            0x10		// Delay before SPCK. (0x40)
#define SPI_DELAY_BETWEEN           0x00		// Delay between consecutive transfers. (0x10)
#define SPI_BAUD_RATE               30000000	// SPI clock speed.


// Prototypes of local functions
int xtio_set_spi_mode(void * spi_handle, xtio_spi_mode_t xtio_spi_mode);
int xtio_get_spi_mode(void * spi_handle, xtio_spi_mode_t * xtio_spi_mode);
int xtio_spi_write(void * spi_handle, uint8_t * write_buffer, uint32_t number_of_bytes_to_write);
int xtio_spi_read(void * spi_handle, uint8_t * read_buffer, uint32_t number_of_bytes_to_read);
int xtio_spi_write_read(void * spi_handle, uint8_t * write_buffer, uint32_t number_of_bytes_to_write, uint8_t * read_buffer, uint32_t number_of_bytes_to_read);

/**
 * [set_pin_peripheral_mode description]
 * @todo Use the xtio_gpio functions instead
 */
void set_pin_peripheral_mode(ioport_pin_t pin,ioport_mode_t mode);
void set_pin_peripheral_mode(ioport_pin_t pin,ioport_mode_t mode)
{
	ioport_disable_pin(pin);
	ioport_set_pin_mode(pin, mode);
}


int xtio_spi_init(
    xtio_spi_handle_t ** spi_handle,     ///<
    void* instance_memory,
    xtio_spi_callbacks_t * spi_callbacks,
    void * user_reference,
    xtio_spi_mode_t default_xtio_spi_mode
)
{
	int status = XT_SUCCESS;
	xtio_spi_handle_t * spi_handle_local = (xtio_spi_handle_t *)instance_memory;
	memset(spi_handle_local, 0, sizeof(xtio_spi_handle_t));
    spi_handle_local->user_reference = user_reference;
    if (spi_callbacks == NULL)
    {
        spi_handle_local->set_spi_mode = xtio_set_spi_mode;
        spi_handle_local->get_spi_mode = xtio_get_spi_mode;
        spi_handle_local->spi_write = xtio_spi_write;
        spi_handle_local->spi_read = xtio_spi_read;
        spi_handle_local->spi_write_read = xtio_spi_write_read;
    }


	qspi_disable(QSPI);
	sysclk_enable_peripheral_clock(ID_QSPI);
	// Radar
	/**
	 * @todo Rewrite to use the qspi_set_config function since most of the
	 *       functions below are local to the qspi.c
	 */
	qspi_set_clock_polarity(QSPI, SPI_CLOCK_POLARITY);
	qspi_set_clock_phase(QSPI, SPI_CLOCK_PHASE);
	qspi_set_bits_per_transfer(QSPI, SPI_CSR_BITS_8_BIT);
	qspi_set_baudrate(QSPI, SPI_BAUD_RATE);
	qspi_set_transfer_delay(QSPI, SPI_DELAY_BEFORE);
	qspi_set_delay_between_consecutive_transfers(QSPI, SPI_DELAY_BETWEEN);
	qspi_set_chip_select_mode(QSPI, 1);
	qspi_set_memory_mode(QSPI);// needed for quad mode spi
	qspi_enable(QSPI);
		// Configure SPI pins
	set_pin_peripheral_mode(QSPI3_GPIO, QSPI3_GPIO_FLAGS);
	set_pin_peripheral_mode(QSPI2_GPIO, QSPI2_GPIO_FLAGS);
	set_pin_peripheral_mode(SPI_MISO_GPIO, SPI_MISO_FLAGS);
	set_pin_peripheral_mode(SPI_MOSI_GPIO, SPI_MOSI_FLAGS);
	set_pin_peripheral_mode(SPI_SPCK_GPIO, SPI_SPCK_FLAGS);

	// NVA radar chip select
	set_pin_peripheral_mode(NVA_SPI_RADAR_NPCS_GPIO, NVA_SPI_RADAR_NPCS_FLAGS);
	/*
	ioport_set_pin_dir(XPIN_X4ENABLE, IOPORT_DIR_INPUT);
	//##KIH ioport_set_pin_dir(XPIN_TM, IOPORT_DIR_INPUT);
	ioport_set_pin_dir(XPIN_X4IO1,IOPORT_DIR_INPUT);
	ioport_set_pin_dir(XPIN_X4IO2,IOPORT_DIR_INPUT);

    xtio_set_pin_mode(XTIO_X4_ENABLE, XTIO_PULL_DOWN);
    xtio_set_pin_mode(XTIO_X4_IO1, XTIO_PULL_DOWN);
    xtio_set_pin_mode(XTIO_X4_IO2, XTIO_PULL_DOWN);
	ioport_enable_pin(XPIN_X4ENABLE);

	//##KIH ioport_enable_pin(XPIN_ENABLE_MCU02);
	//##KIH ioport_enable_pin(XPIN_TM);
	//##KIH ioport_enable_pin(XPIN_GPIO1);
	//##KIH ioport_enable_pin(XPIN_GPIO2);


	ioport_set_pin_dir(XPIN_IO8,         IOPORT_DIR_OUTPUT); // qspi_cs select 0
	ioport_set_pin_dir(XPIN_IO9,         IOPORT_DIR_OUTPUT);// qspi_cs select 1
	ioport_set_pin_level(XPIN_IO8,false);// radar cs
	ioport_set_pin_level(XPIN_IO9,true); // radar cs

*/

    *spi_handle = spi_handle_local;

	return status;
}

uint32_t xtio_spi_get_instance_size(void)
{
    uint32_t total_size = 0;
    total_size += sizeof(xtio_spi_handle_t);
    // total_size += sizeof(spi_if_t);
    return total_size;
}

int xtio_set_spi_mode(void * spi_handle, xtio_spi_mode_t xtio_spi_mode)
{
    uint32_t status = XT_SUCCESS;

    return status;
}

int xtio_get_spi_mode(void * spi_handle, xtio_spi_mode_t * xtio_spi_mode)
{
    uint32_t status = XT_SUCCESS;

    return status;
}

int xtio_spi_write(void * spi_handle, uint8_t * write_buffer, uint32_t number_of_bytes_to_write)
{
    uint32_t status = XT_SUCCESS;

    return status;
}

int xtio_spi_read(void * spi_handle, uint8_t * read_buffer, uint32_t number_of_bytes_to_read)
{
    uint32_t status = XT_SUCCESS;

    return status;
}

/*
int xtio_spi_write_read(void * spi_handle, uint8_t write_buffer, uint32_t number_of_bytes_to_write, uint8_t read_buffer, uint32_t number_of_bytes_to_read)
{
    uint32_t status = XT_SUCCESS;

    return status;
}
*/

/*
int xtio_spi_write_read(int chipSelect, uint8_t * writeBuffer, int writeLength, uint8_t * readBuffer, int readLength)
{
	// General function for writing data to the SPI bus with the possibility to choose chip select
	// Chip select can be set from 0 to 15, depending on the IO-module used and what IO port is connected to chip select
	// Assuming that chip select was set correctly and inverting the bit number given in chip select
	int status = XT_SUCCESS;
	// Select the Novelda chip.
	//qspi_set_peripheral_chip_select_value(freertos_spi, (~(1 << chipSelect)));
	//qspi
	status = commonSendReciveQSPI(writeBuffer,writeLength,readBuffer,readLength);
	return status;
}
*/

struct qspi_inst_frame_t qspi_frame = {{0},0};
uint32_t use_quad_spi = 0;
struct qspid_t g_qspid = {QSPI, {0}, {0}, 0};


__attribute__ ((optimize("-O0"))) int xtio_spi_write_read(void * spi_handle, uint8_t * write_buffer, uint32_t number_of_bytes_to_write, uint8_t * read_buffer, uint32_t number_of_bytes_to_read)
{
	int status = XT_SUCCESS;

	if ((0 == number_of_bytes_to_read) && (0 == number_of_bytes_to_write))
	{
		return status;
	}

	enum qspi_access rw = QSPI_READ_ACCESS;
	qspi_frame.inst_frame.bm.b_data_en = 1;
	qspi_frame.inst_frame.bm.b_inst_en = 1;

	qspi_frame.inst_frame.bm.b_addr_en = 0;
	qspi_frame.inst_frame.bm.b_opt_en = 0;

	qspi_frame.inst_frame.bm.b_dummy_cycles = 0;
	if (use_quad_spi == 0)
	{
		qspi_frame.inst_frame.bm.b_width = QSPI_IFR_WIDTH_SINGLE_BIT_SPI;
	}
	else
	{
		qspi_frame.inst_frame.bm.b_width = QSPI_IFR_WIDTH_QUAD_CMD;
	}

	qspi_frame.inst_frame.bm.b_tfr_type = (QSPI_IFR_TFRTYP_TRSFR_READ >> QSPI_IFR_TFRTYP_Pos);
	g_qspid.qspi_frame = &qspi_frame;
	g_qspid.qspi_command.option = 0;

	if(number_of_bytes_to_read == 0)
	{
		rw = QSPI_WRITE_ACCESS;
		qspi_frame.inst_frame.bm.b_tfr_type = (QSPI_IFR_TFRTYP_TRSFR_WRITE>> QSPI_IFR_TFRTYP_Pos);
		if(number_of_bytes_to_write == 1)
		{
			rw = QSPI_CMD_ACCESS;
			qspi_frame.inst_frame.bm.b_tfr_type  = (QSPI_IFR_TFRTYP_TRSFR_READ >> QSPI_IFR_TFRTYP_Pos);
			qspi_frame.inst_frame.bm.b_data_en = 0;
		}
	}

	if(0 != number_of_bytes_to_write)
	{
		g_qspid.qspi_command.instruction = write_buffer[0];

		// WARNING: g_qspid.qspi_buffer.data_tx is defined as a buffer of utint32_t values,
		//          but internally in ASF it is used as a uint8_t buffer
		g_qspid.qspi_buffer.data_tx = (uint32_t *)&write_buffer[1];
		g_qspid.qspi_buffer.tx_data_size = number_of_bytes_to_write-1;
	}
	// WARNING: g_qspid.qspi_buffer.data_rx is defined as a buffer of utint32_t values,
	//          but internally in ASF it is used as a uint8_t buffer
	g_qspid.qspi_buffer.data_rx = (uint32_t *)read_buffer;
	g_qspid.qspi_buffer.rx_data_size = number_of_bytes_to_read;
	//qspi_flash_access_memory(&g_qspid,rw,0);
	qspi_flash_execute_command(&g_qspid,rw);

	return status;
}
