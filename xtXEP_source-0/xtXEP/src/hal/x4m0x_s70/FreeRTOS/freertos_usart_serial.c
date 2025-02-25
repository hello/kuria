/**
 * \file
 *
 * \brief FreeRTOS Peripheral Control API For the USART
 *
 * Copyright (c) 2012-2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/* Standard includes. */
#include <string.h>

/* ASF includes. */
#include "serial.h"
#include "xdmac.h"
#include "freertos_usart_serial.h"
#include "freertos_peripheral_control_private.h"
#include "freertos_xdma.h"
#include "xep_hal.h"

/* Every bit in the interrupt mask. */
#define MASK_ALL_INTERRUPTS         (0xffffffffUL)

/* Interrupts that are enabled to catch error conditions on the bus. */
#define SR_ERROR_INTERRUPTS         (US_CSR_OVRE | US_CSR_FRAME | US_CSR_PARE)
#define IER_ERROR_INTERRUPTS        (US_IER_OVRE | US_IER_FRAME | US_IER_PARE)

/* A value written to a member of an Rx buffer to show that the Rx side was not
initialised and therefore should not be used. */
#define RX_NOT_USED                 (uint8_t *) 0x1

/* Divide by this number to convert bits per second to bits per 5 milliseconds.
Equivalent to times 5 then divide by 1000. */
#define BITS_PER_5_MS               (200UL)







/* Work out how many USARTS are present. */
#if defined(XDAMC_CHANNEL_HWID_USART2_TX)
	#define MAX_USARTS                              (3)
#elif defined(XDAMC_CHANNEL_HWID_USART1_TX)
	#define MAX_USARTS                              (2)
#elif defined(XDAMC_CHANNEL_HWID_USART0_TX)
	#define MAX_USARTS                              (1)
#else
	#error No PDC USARTS defined
#endif


#define US_IER_ENDRX							(0x1)
#define US_IER_ENDTX							(0x1)
#define US_IDR_ENDRX							(0x1)
#define US_IDR_ENDTX							(0x1)
#define US_CSR_ENDRX							(0x1)
#define US_CSR_ENDTX							(0x1)
#define PDC_UART0								(20)


enum buffer_operations {
	data_added = 0,
	data_removed
};

/* Configures the Rx DMA to receive data into free space within the Rx buffer. */
static void configure_rx_dma(uint32_t usart_index,
		enum buffer_operations operation_performed);

/* A common interrupt handler called by all the USART peripherals. */
static void local_usart_handler(const portBASE_TYPE usart_index);

/* Structures to manage the DMA control for both Rx and Tx transactions. */
static freertos_pdc_rx_control_t rx_buffer_definitions[MAX_USARTS];
static freertos_dma_event_control_t rx_dma_control[MAX_USARTS];
static freertos_dma_event_control_t tx_dma_control[MAX_USARTS];

/* Create an array that holds the information required about each defined
USART. */
static const freertos_pdc_peripheral_parameters_t all_usart_definitions[MAX_USARTS] = {
#if	(MAX_USARTS > 0)
	{USART0, XDAMC_CHANNEL_HWID_USART0_TX, ID_USART0, USART0_IRQn},
#endif

#if (MAX_USARTS > 1)
	{USART1, XDAMC_CHANNEL_HWID_USART1_TX, ID_USART1, USART1_IRQn},
#endif

#if (MAX_USARTS > 2)
	{USART2, XDAMC_CHANNEL_HWID_USART2_TX, ID_USART2, USART2_IRQn},
#endif
};

#define XDMA_CH_USART_SPI_RX								(XDAMC_CHANNEL_HWID_USART2_RX)
#define XDMA_CH_USART_SPI_TX								(XDAMC_CHANNEL_HWID_USART2_TX)

static void xdma_usart2_rx_done(void)
{
	portBASE_TYPE higher_priority_task_woken = pdFALSE;
	
	uint32_t dma_status = xdmac_channel_get_interrupt_status(XDMAC, XDMA_CH_USART_SPI_RX);

	/* Has the PDC completed a transmission? */
	if ((dma_status & XDMAC_CIS_BIS) != 0UL) {
		xdmac_channel_disable_interrupt(XDMAC, XDMA_CH_USART_SPI_RX, MASK_ALL_INTERRUPTS);
		xdmac_channel_disable(XDMAC, XDMA_CH_USART_SPI_RX);
		xdmac_disable_interrupt(XDMAC, (1 << XDMA_CH_USART_SPI_RX));
		
		xdmac_channel_disable(XDMAC, XDMA_CH_USART_SPI_RX);
		
		/* If the driver is supporting multi-threading, then return the access
		mutex.  NOTE: As a reception is performed by first performing a
		transmission, the SPI receive function uses the tx access semaphore. */
		if (tx_dma_control[2].peripheral_access_mutex != NULL) {
			xSemaphoreGiveFromISR(tx_dma_control[2].peripheral_access_mutex,
					&higher_priority_task_woken);
		}

		/* If the receiving task supplied a notification semaphore, then
		notify the task that the transmission has completed. */
		if (rx_dma_control[2].transaction_complete_notification_semaphore != NULL) {
			xSemaphoreGiveFromISR(rx_dma_control[2].transaction_complete_notification_semaphore,
					&higher_priority_task_woken);
		}
	} else {
		__NOP();
	}
	__DMB();

	/* If giving a semaphore caused a task to unblock, and the unblocked task
	has a priority equal to or higher than the currently running task (the task
	this ISR interrupted), then higher_priority_task_woken will have
	automatically been set to pdTRUE within the semaphore function.
	portEND_SWITCHING_ISR() will then ensure that this ISR returns directly to
	the higher priority unblocked task. */
	portEND_SWITCHING_ISR(higher_priority_task_woken);
}

static void xdma_usart2_tx_done(void)
{
	portBASE_TYPE higher_priority_task_woken = pdFALSE;
	
	uint32_t dma_status = xdmac_channel_get_interrupt_status(XDMAC, XDMA_CH_USART_SPI_TX);

	/* Has the PDC completed a transmission? */
	if ((dma_status & XDMAC_CIS_BIS) != 0UL) {
		xdmac_channel_disable_interrupt(XDMAC, XDMA_CH_USART_SPI_TX, MASK_ALL_INTERRUPTS);
		xdmac_channel_disable(XDMAC, XDMA_CH_USART_SPI_TX);
		xdmac_disable_interrupt(XDMAC, (1 << XDMA_CH_USART_SPI_TX));
		
		/* If the driver is supporting multi-threading, then return the access
		mutex. */
		if (tx_dma_control[2].peripheral_access_mutex != NULL) {
			xSemaphoreGiveFromISR(tx_dma_control[2].peripheral_access_mutex,
					&higher_priority_task_woken);
		}

		/* if the sending task supplied a notification semaphore, then
		 * notify the task that the transmission has completed. */
		if (tx_dma_control[2].transaction_complete_notification_semaphore != NULL) {
			xSemaphoreGiveFromISR(tx_dma_control[2].transaction_complete_notification_semaphore,
					&higher_priority_task_woken);
		}
	} else {
		__NOP();
	}
	__DMB();

	/* If giving a semaphore caused a task to unblock, and the unblocked task
	has a priority equal to or higher than the currently running task (the task
	this ISR interrupted), then higher_priority_task_woken will have
	automatically been set to pdTRUE within the semaphore function.
	portEND_SWITCHING_ISR() will then ensure that this ISR returns directly to
	the higher priority unblocked task. */
	portEND_SWITCHING_ISR(higher_priority_task_woken);
}


freertos_usart_if freertos_usart_spi_init(Usart *p_usart,
		const usart_spi_opt_t *const usart_spi_settings,
		const freertos_peripheral_options_t *const freertos_driver_parameters)
{
	portBASE_TYPE usart_index;
	freertos_usart_if return_value;
	
	xdmac_disable_interrupt(XDMAC, (1 << XDMA_CH_USART_SPI_RX) | (1 << XDMA_CH_USART_SPI_TX));
	xdmac_channel_disable(XDMAC, XDMA_CH_USART_SPI_RX);
	xdmac_channel_disable(XDMAC, XDMA_CH_USART_SPI_TX);
	
	/* Disable everything before enabling the clock. */
	usart_index = get_pdc_peripheral_details(all_usart_definitions,
			MAX_USARTS,
			(void *) p_usart);
	usart_disable_tx(p_usart);
	usart_disable_rx(p_usart);
	pmc_enable_periph_clk(
			all_usart_definitions[usart_index].peripheral_id);
	if (freertos_driver_parameters->operation_mode == SPI_SLAVE)
	{
		usart_init_spi_slave(p_usart,usart_spi_settings);
	}
	else if (freertos_driver_parameters->operation_mode == SPI_MASTER)
	{
		usart_init_spi_master(p_usart,usart_spi_settings,sysclk_get_cpu_hz());
	}
	else
	{
		return NULL;
	}
	usart_disable_interrupt(p_usart, MASK_ALL_INTERRUPTS);	
	create_peripheral_control_semaphores(( WAIT_TX_COMPLETE | WAIT_RX_COMPLETE),
	&tx_dma_control[2],
	&rx_dma_control[2]);
	
	


	//register callbacks
	freertos_xdma_register(XDMA_CH_USART_SPI_RX, xdma_usart2_rx_done);
	freertos_xdma_register(XDMA_CH_USART_SPI_TX, xdma_usart2_tx_done);
	/* Finally, enable the receiver and transmitter. */
	usart_enable_tx(p_usart);
	usart_enable_rx(p_usart);
	

	return_value = (freertos_usart_if) p_usart;
	
	return return_value;
		
}

#include "ioport.h"
status_code_t freertos_usart_spi_write_packet_async(freertos_usart_if p_spi,
		const uint8_t *data, size_t len, TickType_t block_time_ticks,
		SemaphoreHandle_t notification_semaphore)
{
	status_code_t return_value;
	xdmac_channel_config_t tx_packet;
	
	return_value = freertos_obtain_peripheral_access_mutex(&tx_dma_control[2], &block_time_ticks);

	if (return_value == STATUS_OK) {
		freertos_start_pdc_tx(&tx_dma_control, notification_semaphore);
		/** @todo KIH 2016-09-14: Check it freertos_start_pdc_tx(&tx_dma_control, notification_semaphore); is correct - gives compiler warning
			Should it be &tx_dma_control[0] or tx_dma_control ??
		*/
		
		irqflags_t status = cpu_irq_save();
		
		tx_packet.mbr_ubc = len;

		tx_packet.mbr_sa = (uint32_t)data;
		tx_packet.mbr_da = (uint32_t)&(USART2->US_THR);
		tx_packet.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
		XDMAC_CC_MBSIZE_SINGLE |
		XDMAC_CC_DSYNC_MEM2PER |
		XDMAC_CC_CSIZE_CHK_1 |
		XDMAC_CC_DWIDTH_BYTE |
		XDMAC_CC_SIF_AHB_IF0 |
		XDMAC_CC_DIF_AHB_IF1 |
		XDMAC_CC_SAM_INCREMENTED_AM |
		XDMAC_CC_DAM_FIXED_AM |
		XDMAC_CC_PERID(XDAMC_CHANNEL_HWID_USART2_TX);

		tx_packet.mbr_bc = 0;
		tx_packet.mbr_ds =  0;
		tx_packet.mbr_sus = 0;
		tx_packet.mbr_dus = 0;

		xdmac_configure_transfer(XDMAC, XDMA_CH_USART_SPI_TX, &tx_packet);
		xdmac_channel_set_descriptor_control(XDMAC, XDMA_CH_USART_SPI_TX, 0);
		xdmac_channel_enable_interrupt(XDMAC, XDMA_CH_USART_SPI_TX, XDMAC_CIE_BIE |
		XDMAC_CIE_RBIE  |
		XDMAC_CIE_WBIE  |
		XDMAC_CIE_ROIE);

		/*Start*/
		xdmac_channel_get_interrupt_status(XDMAC, XDMA_CH_USART_SPI_TX);
		xdmac_channel_get_interrupt_status(XDMAC, XDMA_CH_USART_SPI_TX);
		xdmac_channel_get_interrupt_status(XDMAC, XDMA_CH_USART_SPI_TX);
		xdmac_enable_interrupt(XDMAC, XDMA_CH_USART_SPI_TX);
		xdmac_channel_enable(XDMAC, XDMA_CH_USART_SPI_TX);

		__DMB();
		
		cpu_irq_restore(status);

		return_value = freertos_optionally_wait_transfer_completion(&tx_dma_control[2],
		notification_semaphore, block_time_ticks);
		} 
		else 
		{
			return_value = ERR_INVALID_ARG;
		}
	
	return return_value;
}


status_code_t freertos_usart_spi_read_write_packet_async(freertos_usart_if p_spi,
		uint8_t *data, uint32_t len,uint8_t *data_write, uint32_t len_write, TickType_t block_time_ticks,
		SemaphoreHandle_t notification_semaphore)
{
	status_code_t return_value;
	xdmac_channel_config_t rx_packet, tx_packet;
	//##KIH volatile uint16_t junk_value;

	if (len == 0) {
		return STATUS_OK;
	}

	/* Because the peripheral is half duplex, there is only one access mutex
	   and the rx uses the tx mutex. */
	return_value = freertos_obtain_peripheral_access_mutex(&tx_dma_control[2], &block_time_ticks);

	if (return_value == STATUS_OK) {
		/* Data must be sent for data to be received.  Set the receive
		buffer to all 0xffs so it can also be used as the send buffer. */
		
	uint32_t tmp = 0;
		if(usart_is_rx_ready(USART2))
			usart_read(USART2,&tmp);
		
		
		

		/* Start the PDC reception, although nothing is received until the
		   SPI is also transmitting. */
		freertos_start_pdc_rx(&rx_dma_control[2], notification_semaphore);

		/*Channel configuration*/
		irqflags_t status = cpu_irq_save();
		tx_packet.mbr_ubc = len_write;

		tx_packet.mbr_sa = (uint32_t)data_write;
		tx_packet.mbr_da = (uint32_t)&(USART2->US_THR);
		tx_packet.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
					XDMAC_CC_MBSIZE_SINGLE |
					XDMAC_CC_DSYNC_MEM2PER |
					XDMAC_CC_CSIZE_CHK_1 |
					XDMAC_CC_DWIDTH_BYTE |
					XDMAC_CC_SIF_AHB_IF0 |
					XDMAC_CC_DIF_AHB_IF1 |
					XDMAC_CC_SAM_INCREMENTED_AM |
					XDMAC_CC_DAM_FIXED_AM |
					XDMAC_CC_PERID(XDAMC_CHANNEL_HWID_USART2_TX);

		tx_packet.mbr_bc = 0;
		tx_packet.mbr_ds =  0;
		tx_packet.mbr_sus = 0;
		tx_packet.mbr_dus = 0;
//changed from RX to TX
		xdmac_configure_transfer(XDMAC, XDMA_CH_USART_SPI_TX, &tx_packet);
		xdmac_channel_set_descriptor_control(XDMAC, XDMA_CH_USART_SPI_TX, 0);
			
		rx_packet.mbr_ubc = len;
		rx_packet.mbr_da = (uint32_t)data;
		rx_packet.mbr_sa = (uint32_t)&USART2->US_RHR;
		rx_packet.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
					XDMAC_CC_MBSIZE_SINGLE |
					XDMAC_CC_DSYNC_PER2MEM |
					XDMAC_CC_CSIZE_CHK_1 |
					XDMAC_CC_DWIDTH_BYTE|
					XDMAC_CC_SIF_AHB_IF1 |
					XDMAC_CC_DIF_AHB_IF0 |
					XDMAC_CC_SAM_FIXED_AM |
					XDMAC_CC_DAM_INCREMENTED_AM |
					XDMAC_CC_PERID(XDAMC_CHANNEL_HWID_USART2_RX);

		rx_packet.mbr_bc = 0;
		rx_packet.mbr_ds =  0;
		rx_packet.mbr_sus = 0;
		rx_packet.mbr_dus = 0;

		xdmac_configure_transfer(XDMAC, XDMA_CH_USART_SPI_RX, &rx_packet);
		xdmac_channel_set_descriptor_control(XDMAC, XDMA_CH_USART_SPI_RX, 0);
		xdmac_channel_get_interrupt_status(XDMAC, XDMA_CH_USART_SPI_RX);
		xdmac_channel_enable_interrupt(XDMAC, XDMA_CH_USART_SPI_RX, XDMAC_CIE_BIE |
					XDMAC_CIE_RBIE  |
					XDMAC_CIE_WBIE  |
					XDMAC_CIE_ROIE);
		xdmac_enable_interrupt(XDMAC, XDMA_CH_USART_SPI_RX);
		//xdmac_channel_enable(XDMAC, XDMA_CH_USART_SPI_RX);
			
		/*Start*/
		xdmac_enable_interrupt(XDMAC, XDMA_CH_USART_SPI_TX);
		xdmac_channel_enable(XDMAC, XDMA_CH_USART_SPI_RX);
		xdmac_channel_enable(XDMAC, XDMA_CH_USART_SPI_TX);
		//sync mechanism between x4 and m7
		//x4 will send when gpio2 is low
		
		xtio_set_direction(XTIO_X4_IO2, XTIO_OUTPUT, XTIO_PIN_LEVEL_LOW);

		__DMB();
		cpu_irq_restore(status);

		return_value = freertos_optionally_wait_transfer_completion(&rx_dma_control[2],
				notification_semaphore, block_time_ticks);
	} else {
		return_value = ERR_INVALID_ARG;
	}

	return return_value;
}


status_code_t freertos_usart_spi_read_packet_async(freertos_usart_if p_spi,
		uint8_t *data, uint32_t len, TickType_t block_time_ticks,
		SemaphoreHandle_t notification_semaphore)
{
	status_code_t return_value;
	xdmac_channel_config_t rx_packet, tx_packet;
	//##KIH volatile uint16_t junk_value;

	if (len == 0) {
		return STATUS_OK;
	}

	/* Because the peripheral is half duplex, there is only one access mutex
	   and the rx uses the tx mutex. */
	return_value = freertos_obtain_peripheral_access_mutex(&tx_dma_control[2], &block_time_ticks);

	if (return_value == STATUS_OK) {
		/* Data must be sent for data to be received.  Set the receive
		buffer to all 0xffs so it can also be used as the send buffer. */
		//memset((void *)data, 0xff, (size_t)len); // OJE: Removed in order to send while receiving.

		/* Ensure Rx is already empty. */
		uint32_t tmp = 0;
		if(usart_is_rx_ready(USART2))
			usart_read(USART2,&tmp);
		/*while(usart_is_rx_buf_full(USART2))
		{
			
		}*/
		
		//while(qspi_is_rx_full(QSPI) != 0) {
		//	junk_value = QSPI->QSPI_RDR;
		//	(void) junk_value;
		//}

		/* Start the PDC reception, although nothing is received until the
		   SPI is also transmitting. */
		freertos_start_pdc_rx(&rx_dma_control[2], notification_semaphore);

		/*Channel configuration*/
		irqflags_t status = cpu_irq_save();
		tx_packet.mbr_ubc = len;

		tx_packet.mbr_sa = (uint32_t)data;
		tx_packet.mbr_da = (uint32_t)&(USART2->US_THR);
		tx_packet.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
					XDMAC_CC_MBSIZE_SINGLE |
					XDMAC_CC_DSYNC_MEM2PER |
					XDMAC_CC_CSIZE_CHK_1 |
					XDMAC_CC_DWIDTH_BYTE |
					XDMAC_CC_SIF_AHB_IF0 |
					XDMAC_CC_DIF_AHB_IF1 |
					XDMAC_CC_SAM_INCREMENTED_AM |
					XDMAC_CC_DAM_FIXED_AM |
					XDMAC_CC_PERID(XDAMC_CHANNEL_HWID_USART2_TX);

		tx_packet.mbr_bc = 0;
		tx_packet.mbr_ds =  0;
		tx_packet.mbr_sus = 0;
		tx_packet.mbr_dus = 0;

		xdmac_configure_transfer(XDMAC, XDMA_CH_USART_SPI_TX, &tx_packet);
		xdmac_channel_set_descriptor_control(XDMAC, XDMA_CH_USART_SPI_TX, 0);
			
		rx_packet.mbr_ubc = len;
		rx_packet.mbr_da = (uint32_t)data;
		rx_packet.mbr_sa = (uint32_t)&USART2->US_RHR;
		rx_packet.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
					XDMAC_CC_MBSIZE_SINGLE |
					XDMAC_CC_DSYNC_PER2MEM |
					XDMAC_CC_CSIZE_CHK_1 |
					XDMAC_CC_DWIDTH_BYTE|
					XDMAC_CC_SIF_AHB_IF1 |
					XDMAC_CC_DIF_AHB_IF0 |
					XDMAC_CC_SAM_FIXED_AM |
					XDMAC_CC_DAM_INCREMENTED_AM |
					XDMAC_CC_PERID(XDAMC_CHANNEL_HWID_USART2_RX);

		rx_packet.mbr_bc = 0;
		rx_packet.mbr_ds =  0;
		rx_packet.mbr_sus = 0;
		rx_packet.mbr_dus = 0;

		xdmac_configure_transfer(XDMAC, XDMA_CH_USART_SPI_RX, &rx_packet);
		xdmac_channel_set_descriptor_control(XDMAC, XDMA_CH_USART_SPI_RX, 0);
		xdmac_channel_get_interrupt_status(XDMAC, XDMA_CH_USART_SPI_RX);
		xdmac_channel_enable_interrupt(XDMAC, XDMA_CH_USART_SPI_RX, XDMAC_CIE_BIE |
					XDMAC_CIE_RBIE  |
					XDMAC_CIE_WBIE  |
					XDMAC_CIE_ROIE);
		xdmac_enable_interrupt(XDMAC, XDMA_CH_USART_SPI_RX);
		xdmac_enable_interrupt(XDMAC, XDMA_CH_USART_SPI_TX);
		xdmac_channel_enable(XDMAC, XDMA_CH_USART_SPI_RX);
			
		/*Start*/
		
		xdmac_channel_enable(XDMAC, XDMA_CH_USART_SPI_TX);

		__DMB();
		cpu_irq_restore(status);

		return_value = freertos_optionally_wait_transfer_completion(&rx_dma_control[2],
				notification_semaphore, block_time_ticks);
	} else {
		return_value = ERR_INVALID_ARG;
	}

	return return_value;
}



/**
 * \ingroup freertos_usart_peripheral_control_group
 * \brief Initializes the FreeRTOS ASF USART driver for the specified USART
 * port.
 *
 * freertos_usart_serial_init() is an ASF specific FreeRTOS driver function.  It
 * must be called before any other ASF specific FreeRTOS driver functions
 * attempt to access the same USART port.
 *
 * If freertos_driver_parameters->operation_mode equals USART_RS232 then
 * freertos_usart_serial_init() will configure the USART port for standard RS232
 * operation.  If freertos_driver_parameters->operation_mode equals any other
 * value then freertos_usart_serial_init() will not take any action.
 *
 * Other ASF USART functions can be called after freertos_usart_serial_init()
 * has completed successfully.
 *
 * The FreeRTOS ASF driver both installs and handles the USART PDC interrupts.
 * Users do not need to concern themselves with interrupt handling, and must
 * not install their own interrupt handler.
 *
 * This driver is provided with an application note, and an example project that
 * demonstrates the use of this function.
 *
 * \param p_usart    The USART peripheral being initialized.
 * \param uart_parameters    Structure that defines the USART bus and transfer
 *     parameters, such the baud rate and the number of data bits.
 *     sam_usart_opt_t is a standard ASF type (it is not FreeRTOS specific).
 * \param freertos_driver_parameters    Defines the driver behavior.  See the
 *    freertos_peripheral_options_t documentation, and the application note that
 *    accompanies the ASF specific FreeRTOS functions.
 *
 * \return If the initialization completes successfully then a handle that can
 *     be used with FreeRTOS USART read and write functions is returned.  If
 *     the initialisation fails then NULL is returned.
 */
freertos_usart_if freertos_usart_serial_init(Usart *p_usart,
		const sam_usart_opt_t *const uart_parameters,
		const freertos_peripheral_options_t *const freertos_driver_parameters)
{
	portBASE_TYPE usart_index;
	bool is_valid_operating_mode;
	freertos_usart_if return_value;
	const enum peripheral_operation_mode valid_operating_modes[] = {USART_RS232};

	/* Find the index into the all_usart_definitions array that holds details of
	the p_usart peripheral. */
	usart_index = get_pdc_peripheral_details(all_usart_definitions,
			MAX_USARTS,
			(void *) p_usart);

	/* Check the requested operating mode is valid for the peripheral. */
	is_valid_operating_mode = check_requested_operating_mode(
			freertos_driver_parameters->operation_mode,
			valid_operating_modes,
			sizeof(valid_operating_modes) /
			sizeof(enum peripheral_operation_mode));

	/* Don't do anything unless a valid p_usart pointer was used, and a valid
	operating mode was requested. */
	if ((usart_index < MAX_USARTS) && (is_valid_operating_mode == true)) {
		/* This function must be called exactly once per supported USART.  Check it
		has not been called	before. */
		configASSERT(rx_buffer_definitions[usart_index].next_byte_to_read == NULL);

		/* Disable everything before enabling the clock. */
		usart_disable_tx(p_usart);
		usart_disable_rx(p_usart);
		
#if SAM4E
		pdc_disable_transfer(all_usart_definitions[usart_index].pdc_base_address,
				(PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS));
#else
#endif
		/* Enable the peripheral clock in the PMC. */
		pmc_enable_periph_clk(
				all_usart_definitions[usart_index].peripheral_id);

		switch (freertos_driver_parameters->operation_mode) {
		case USART_RS232:
			/* Call the standard ASF init function. */
			usart_init_rs232(p_usart, uart_parameters,
					sysclk_get_cpu_hz());
			break;		
			

		default:
			/* Other modes are not currently supported. */
			break;
		}

		/* Disable all the interrupts. */
		usart_disable_interrupt(p_usart, MASK_ALL_INTERRUPTS);

		/* Create any required peripheral access mutexes and transaction complete
		semaphores.  This peripheral is full duplex so only the Tx semaphores
		are created in the following function.  The the Rx semaphores are
		created	separately. */
		create_peripheral_control_semaphores(
				freertos_driver_parameters->options_flags,
				&(tx_dma_control[usart_index]),
				NULL /* The rx structures are not created in this function. */);

		/* Is the driver also going to receive? */
		if (freertos_driver_parameters->receive_buffer != NULL) {
			/* rx_event_semaphore is used to signal the arival of new data.  It
			must be a counting semaphore for the following reason:  If the Rx
			DMA places data at the end of its circular buffer it will give the
			semaphore to indicate the presence of unread data.  If it then
			receives more data, it will write this to the start of the circular
			buffer, then give the semaphore again.  Now, if a task reads data
			out of the same circular buffer, and requests less data than is
			available, but more than is available between the next read pointer
			and the end of the buffer, the actual amount returned will be
			capped to that available up to the end of the buffer only.  If this
			semaphore was a binary semaphore, it would then be 'taken' even
			though, unknown to the reading task, unread and therefore available
			data remained at the beginning of the buffer. */
			rx_buffer_definitions[usart_index].rx_event_semaphore =
					xSemaphoreCreateCounting(portMAX_DELAY, 0);
			configASSERT(rx_buffer_definitions[usart_index].rx_event_semaphore);

			/* Set the timeout to 5ms, then start waiting for a character (the
			timeout is not started until characters have started to	be
			received). */
			usart_set_rx_timeout(p_usart,
					(uart_parameters->baudrate / BITS_PER_5_MS));
			usart_start_rx_timeout(p_usart);

			/* The receive buffer is currently empty, so the DMA has control
			over the entire buffer. */
			//VKBrx_buffer_definitions[usart_index].rx_pdc_parameters.ul_addr =
			//VKB		(uint32_t)freertos_driver_parameters->receive_buffer;
			//VKBrx_buffer_definitions[usart_index].rx_pdc_parameters.ul_size =
			//VKB		freertos_driver_parameters->receive_buffer_size;
#if SAM4E
			pdc_rx_init(
					all_usart_definitions[usart_index].pdc_base_address,
					&(rx_buffer_definitions[usart_index].rx_pdc_parameters),
					NULL);
#else
#endif
			/* Set the next byte to read to the start of the buffer as no data
			has yet been read. */
			rx_buffer_definitions[usart_index].next_byte_to_read =
					freertos_driver_parameters->receive_buffer;

			/* Remember the limits of entire buffer. */
			//VKBrx_buffer_definitions[usart_index].rx_buffer_start_address =
			//VKB		rx_buffer_definitions[usart_index].rx_pdc_parameters.ul_addr;
			//VKBrx_buffer_definitions[usart_index].past_rx_buffer_end_address =
			//VKB		rx_buffer_definitions[usart_index].rx_buffer_start_address +
			//VKB		freertos_driver_parameters->receive_buffer_size;

			/* If the rx driver is to be thread aware, create an access control
			mutex. */
			if ((freertos_driver_parameters->options_flags &
					USE_RX_ACCESS_MUTEX) != 0) {
				rx_buffer_definitions[usart_index].rx_access_mutex =
					xSemaphoreCreateMutex();
				configASSERT(rx_buffer_definitions[usart_index].rx_access_mutex);
			}

			/* Catch the DMA running out of Rx space, and gaps in the
			reception.  These events are both used to signal that there is
			data available in the Rx buffer. */
			usart_enable_interrupt(p_usart, US_IER_ENDRX | US_IER_TIMEOUT);

			/* The Rx DMA is running all the time, so enable it now. */
#if SAM4E
			pdc_enable_transfer(
					all_usart_definitions[usart_index].pdc_base_address,
					PERIPH_PTCR_RXTEN);
#else
#endif
		} else {
			/* next_byte_to_read is used to check to see if this function
			has been called before, so it must be set to something, even if
			it is not going to be used.  The value it is set to is not
			important, provided it is not zero (NULL). */
			rx_buffer_definitions[usart_index].next_byte_to_read = RX_NOT_USED;
		}

		/* Error interrupts are always enabled. */
		usart_enable_interrupt(
				all_usart_definitions[usart_index].peripheral_base_address,
				IER_ERROR_INTERRUPTS);

		/* Finally, enable the receiver and transmitter. */
		usart_enable_tx(p_usart);
		usart_enable_rx(p_usart);

		return_value = (freertos_usart_if) p_usart;
	} else {
		return_value = NULL;
	}

	return return_value;
}

/**
 * \ingroup freertos_usart_peripheral_control_group
 * \brief Initiate a completely asynchronous multi-byte write operation on a
 * USART peripheral.
 *
 * freertos_usart_write_packet_async() is an ASF specific FreeRTOS driver
 * function.  It configures the USART peripheral DMA controller (PDC) to
 * transmit data on the USART port, then returns.
 * freertos_usart_write_packet_async() does not wait for the transmission to
 * complete before returning.
 *
 * The FreeRTOS USART driver is initialized using a call to
 * freertos_usart_serial_init().  The freertos_driver_parameters.options_flags
 * parameter passed into the initialization function defines the driver behavior.
 * freertos_usart_write_packet_async() can only be used if the
 * freertos_driver_parameters.options_flags parameter passed to the initialization
 * function had the WAIT_TX_COMPLETE bit clear.
 *
 * freertos_usart_write_packet_async() is an advanced function and readers are
 * recommended to also reference the application note and examples that
 * accompany the FreeRTOS ASF drivers.  freertos_usart_write_packet() is a
 * version that does not exit until the PDC transfer is complete, but still
 * allows other RTOS tasks to execute while the transmission is in progress.
 *
 * The FreeRTOS ASF driver both installs and handles the USART PDC interrupts.
 * Users do not need to concern themselves with interrupt handling, and must
 * not install their own interrupt handler.
 *
 * \param p_usart    The handle to the USART peripheral returned by the
 *     freertos_usart_serial_init() call used to initialise the peripheral.
 * \param data    A pointer to the data to be transmitted.
 * \param len    The number of bytes to transmit.
 * \param block_time_ticks    The FreeRTOS ASF USART driver is initialized using
 *     a call to freertos_usart_serial_init().  The
 *     freertos_driver_parameters.options_flags parameter passed to the
 *     initialization function defines the driver behavior.  If
 *     freertos_driver_parameters.options_flags had the USE_TX_ACCESS_MUTEX bit
 *     set, then the driver will only write to the USART peripheral if it has
 *     first gained exclusive access to it.  block_time_ticks specifies the
 *     maximum amount of time the driver will wait to get exclusive access
 *     before aborting the write operation.  Other tasks will execute during any
 *     waiting time.  block_time_ticks is specified in RTOS tick periods.  To
 *     specify a block time in milliseconds, divide the milliseconds value by
 *     portTICK_RATE_MS, and pass the result in block_time_ticks.
 *     portTICK_RATE_MS is defined by FreeRTOS.
 * \param notification_semaphore    The RTOS task that calls the transmit
 *     function exits the transmit function as soon as the transmission starts.
 *     The data being transmitted by the PDC must not be modified until after
 *     the transmission has completed.  The PDC interrupt (handled internally by
 *     the FreeRTOS ASF driver) 'gives' the semaphore when the PDC transfer
 *     completes.  The notification_semaphore therefore provides a mechanism for
 *     the calling task to know when the PDC has finished accessing the data.
 *     The calling task can call standard FreeRTOS functions to block on the
 *     semaphore until the PDC interrupt occurs.  Other RTOS tasks will execute
 *     while the the calling task is in the Blocked state.  The semaphore must
 *     be created using the FreeRTOS vSemaphoreCreateBinary() API function
 *     before it is used as a parameter.
 *
 * \return     ERR_INVALID_ARG is returned if an input parameter is invalid.
 *     ERR_TIMEOUT is returned if block_time_ticks passed before exclusive
 *     access to the USART peripheral could be obtained.  STATUS_OK is returned
 *     if the PDC was successfully configured to perform the USART write
 *     operation.
 */
status_code_t freertos_usart_write_packet_async(freertos_usart_if p_usart,
		const uint8_t *data, size_t len, TickType_t block_time_ticks,
		SemaphoreHandle_t notification_semaphore)
{
	status_code_t return_value;
	portBASE_TYPE usart_index;
	Usart *usart_base;

	usart_base = (Usart *) p_usart;
	usart_index = get_pdc_peripheral_details(all_usart_definitions,
			MAX_USARTS,
			(void *) usart_base);

	/* Don't do anything unless a valid USART pointer was used. */
	if (usart_index < MAX_USARTS) {
		return_value = freertos_obtain_peripheral_access_mutex(
				&(tx_dma_control[usart_index]),
				&block_time_ticks);

		if (return_value == STATUS_OK) {
			//VKBfreertos_start_pdc_tx(&(tx_dma_control[usart_index]),
			//VKB		data, len,
			//VKB		all_usart_definitions[usart_index].pdc_base_address,
			//VKB		notification_semaphore);

			/* Catch the end of transmission so the access mutex can be
			returned, and the task notified (if it supplied a notification
			semaphore).  The interrupt can be enabled here because the ENDTX
			signal from the PDC to the USART will have been de-asserted when
			the next transfer was configured. */
			usart_enable_interrupt(usart_base, US_IER_ENDTX);

			return_value = freertos_optionally_wait_transfer_completion(
					&(tx_dma_control[usart_index]),
					notification_semaphore,
					block_time_ticks);
		}
	} else {
		return_value = ERR_INVALID_ARG;
	}

	return return_value;
}

/**
 * \ingroup freertos_usart_peripheral_control_group
 * \brief Initiate a completely multi-byte read operation on a USART peripheral.
 *
 * The FreeRTOS ASF USART driver uses the PDC to transfer data from a peripheral
 * to a circular buffer.  Reception happens in the background, while the
 * microcontroller is executing application code.* freertos_usart_read_packet()
 * copies bytes from the DMA buffer into the buffer passed as a
 * freertos_usart_read_packet() parameter.
 *
 * Readers are recommended to also reference the application note and examples
 * that accompany the FreeRTOS ASF drivers.
 *
 * The FreeRTOS ASF driver both installs and handles the USART PDC interrupts.
 * Users do not need to concern themselves with interrupt handling, and must
 * not install their own interrupt handler.
 *
 * \param p_usart    The handle to the USART port returned by the
 *     freertos_usart_serial_init() call used to initialise the port.
 * \param data    A pointer to the buffer into which received data is to be
 *     copied.
 * \param len    The number of bytes to copy.
 * \param block_time_ticks    Defines the maximum combined time the function
 *     will wait to get exclusive access to the peripheral and receive the
 *     requested number of bytes.  Other tasks will execute during any waiting
 *     time.
 *
 *     The FreeRTOS ASF USART driver is initialized using a
 *     call to freertos_usart_serial_init().  The
 *     freertos_driver_parameters.options_flags parameter passed to the
 *     initialization function defines the driver behavior.  If
 *     freertos_driver_parameters.options_flags had the USE_RX_ACCESS_MUTEX bit
 *     set, then the driver will only read from the USART buffer if it has
 *     first gained exclusive access to it.  block_time_ticks specifies the
 *     maximum amount of time the driver will wait to get exclusive access
 *     before aborting the read operation.
 *
 *     If the number of bytes available is less than the number requested then
 *     freertos_usart_serial_read_packet() will wait for more bytes to become
 *     available.  block_time_ticks specifies the maximum amount of time the
 *     driver will wait before returning fewer bytes than were requested.
 *
 *     block_time_ticks is specified in RTOS tick periods.  To specify a block
 *     time in milliseconds, divide the milliseconds value by portTICK_RATE_MS,
 *     and pass the result in  block_time_ticks.  portTICK_RATE_MS is defined by
 *     FreeRTOS.
 *
 * \return     The number of bytes that were copied into data.  This will be
 *     less than the requested number of bytes if a time out occurred.
 */
uint32_t freertos_usart_serial_read_packet(freertos_usart_if p_usart,
		uint8_t *data, uint32_t len, TickType_t block_time_ticks)
{
	portBASE_TYPE usart_index, attempt_read;
	Usart *usart_base;
	TimeOut_t time_out_definition;
	uint32_t bytes_read = 0;

	usart_base = (Usart *) p_usart;
	usart_index = get_pdc_peripheral_details(all_usart_definitions,
			MAX_USARTS,
			(void *) usart_base);

	/* It is possible to initialise the peripheral to only use Tx and not Rx.
	Check that Rx has been initialised. */
	configASSERT(rx_buffer_definitions[usart_index].next_byte_to_read);
	configASSERT(rx_buffer_definitions[usart_index].next_byte_to_read !=
			RX_NOT_USED);

	/* Only do anything if the USART is valid. */
	if (usart_index < MAX_USARTS) {
		/* Must not request more bytes than will fit in the buffer. */
		if (len <=
				(rx_buffer_definitions[usart_index].past_rx_buffer_end_address
				- rx_buffer_definitions[usart_index].rx_buffer_start_address)) {
			/* Remember the time on entry. */
			vTaskSetTimeOutState(&time_out_definition);

			/* If an Rx mutex is in use, attempt to obtain it. */
			if (rx_buffer_definitions[usart_index].rx_access_mutex != NULL) {
				/* Attempt to obtain the mutex. */
				attempt_read = xSemaphoreTake(
						rx_buffer_definitions[usart_index].rx_access_mutex,
						block_time_ticks);

				if (attempt_read == pdTRUE) {
					/* The semaphore was obtained, adjust the block_time_ticks to take
					into account the time taken to obtain the semaphore. */
					if (xTaskCheckForTimeOut(&time_out_definition,
							&block_time_ticks) == pdTRUE) {
						attempt_read = pdFALSE;

						/* The port is not going to be used, so return the
						mutex now. */
						xSemaphoreGive(rx_buffer_definitions[usart_index].rx_access_mutex);
					}
				}
			} else {
				attempt_read = pdTRUE;
			}

			if (attempt_read == pdTRUE) {
				do {
					/* Wait until data is available. */
					xSemaphoreTake(rx_buffer_definitions[usart_index].rx_event_semaphore,
							block_time_ticks);

					/* Copy as much data as is available, up to however much
					a maximum of the total number of requested bytes. */
#if SAM4E
					bytes_read += freertos_copy_bytes_from_pdc_circular_buffer(
							&(rx_buffer_definitions[usart_index]),
							all_usart_definitions[usart_index].pdc_base_address->PERIPH_RPR,
							&(data[bytes_read]),
							(len - bytes_read));
#else
#endif
					/* The Rx DMA will have stopped if the Rx buffer had become
					full before this read operation.  If bytes were removed by
					this read then there is guaranteed to be space in the Rx
					buffer and the Rx DMA can be restarted. */
					if (bytes_read > 0) {
						taskENTER_CRITICAL();
						{
							//VKBif(rx_buffer_definitions[usart_index].rx_pdc_parameters.ul_size == 0UL) {
							//VKB	configure_rx_dma(usart_index, data_removed);
							//VKB}
						}
						taskEXIT_CRITICAL();
					}

				  /* Until all the requested bytes are received, or the function
				  runs out of time. */
				} while ((bytes_read < len) && (xTaskCheckForTimeOut(
						&time_out_definition,
						&block_time_ticks) == pdFALSE));

				if (rx_buffer_definitions[usart_index].rx_access_mutex != NULL) {
					/* Return the mutex. */
					xSemaphoreGive(rx_buffer_definitions[usart_index].rx_access_mutex);
				}
			}
		}
	}

	return bytes_read;
}

/*
 * For internal use only.
 * Configures the Rx DMA to receive data into free space within the Rx buffer.
 */
static void configure_rx_dma(uint32_t usart_index,
		enum buffer_operations operation_performed)
{
#if 0
	freertos_pdc_rx_control_t *rx_buffer_definition;

	rx_buffer_definition = &(rx_buffer_definitions[usart_index]);

	/* How much space is there between the start of the DMA buffer and the
	current read pointer?  */

	if (((uint32_t)rx_buffer_definition->next_byte_to_read) ==
			rx_buffer_definition->rx_pdc_parameters.ul_addr) {
		/* The read pointer and the write pointer are equal.  If this function
		was called because data was added to the buffer, then there is no free
		space in the buffer remaining.  If this function was called because data
		was removed from the buffer, then the space remaining is from the write
		pointer up to the end of the buffer. */
		if (operation_performed == data_added) {
			rx_buffer_definition->rx_pdc_parameters.ul_size = 0UL;
		} else {
			rx_buffer_definition->rx_pdc_parameters.ul_size =
				rx_buffer_definition->past_rx_buffer_end_address - rx_buffer_definition->rx_pdc_parameters.ul_addr;
		}
	} else if (((uint32_t)rx_buffer_definition->next_byte_to_read) >
			rx_buffer_definition->rx_pdc_parameters.ul_addr) {
		/* The read pointer is ahead of the write pointer.  The space available
		is up to the write pointer to ensure unread data is not overwritten. */
		rx_buffer_definition->rx_pdc_parameters.ul_size =
			((uint32_t) rx_buffer_definition->next_byte_to_read) - rx_buffer_definition->rx_pdc_parameters.ul_addr;
	} else {
		/* The write pointer is ahead of the read pointer so the space
		available is up to the end of the buffer. */
		rx_buffer_definition->rx_pdc_parameters.ul_size =
			rx_buffer_definition->past_rx_buffer_end_address - rx_buffer_definition->rx_pdc_parameters.ul_addr;
	}

	configASSERT((rx_buffer_definition->rx_pdc_parameters.ul_addr +
			rx_buffer_definition->rx_pdc_parameters.ul_size) <=
			rx_buffer_definition->past_rx_buffer_end_address);

	if (rx_buffer_definition->rx_pdc_parameters.ul_size > 0) {
		/* Restart the DMA to receive into whichever space was calculated
		as remaining.  First clear any characters that might already be in the
		registers. */
#if SAM4E
		pdc_rx_init(
				all_usart_definitions[usart_index].pdc_base_address, &rx_buffer_definition->rx_pdc_parameters,
				NULL);
		pdc_enable_transfer(
				all_usart_definitions[usart_index].pdc_base_address,
				PERIPH_PTCR_RXTEN);
#else
#endif
		usart_enable_interrupt(
				all_usart_definitions[usart_index].peripheral_base_address, US_IER_ENDRX |
				US_IER_TIMEOUT);
	} else {
		/* The write pointer has reached the read pointer.  There is no
		more room so the DMA is not re-enabled until a read has created
		space. */
		usart_disable_interrupt(
				all_usart_definitions[usart_index].peripheral_base_address, US_IER_ENDRX |
				US_IER_TIMEOUT);
	}
#endif
}

/*
 * For internal use only.
 * A common USART interrupt handler that is called for all USART peripherals.
 */
static void local_usart_handler(const portBASE_TYPE usart_index)
{
	portBASE_TYPE higher_priority_task_woken = pdFALSE;
	uint32_t usart_status;
	freertos_pdc_rx_control_t *rx_buffer_definition;

	usart_status = usart_get_status(
			all_usart_definitions[usart_index].peripheral_base_address);
	usart_status &= usart_get_interrupt_mask(
			all_usart_definitions[usart_index].peripheral_base_address);

	rx_buffer_definition = &(rx_buffer_definitions[usart_index]);

	/* Has the PDC completed a transmission? */
	if ((usart_status & US_CSR_ENDTX) != 0UL) {
		usart_disable_interrupt(
				all_usart_definitions[usart_index].peripheral_base_address,
				US_IER_ENDTX);

		/* If the driver is supporting multi-threading, then return the access
		mutex. */
		if (tx_dma_control[usart_index].peripheral_access_mutex != NULL) {
			xSemaphoreGiveFromISR(
					tx_dma_control[usart_index].peripheral_access_mutex,
					&higher_priority_task_woken);
		}

		/* if the sending task supplied a notification semaphore, then
		notify the task that the transmission has completed. */
		if (tx_dma_control[usart_index].transaction_complete_notification_semaphore != NULL) {
			xSemaphoreGiveFromISR(
					tx_dma_control[usart_index].transaction_complete_notification_semaphore,
					&higher_priority_task_woken);
		}
	}

	if ((usart_status & US_CSR_ENDRX) != 0UL) {
		/* It is possible to initialise the peripheral to only use Tx and not Rx.
		Check that Rx has been initialised. */
		configASSERT(rx_buffer_definition->next_byte_to_read);
		configASSERT(rx_buffer_definition->next_byte_to_read !=
				RX_NOT_USED);

		/* Out of DMA buffer, configure the next buffer.  Start by moving
		the DMA buffer start address up to the end of the previously defined
		buffer. */
		//VKBrx_buffer_definition->rx_pdc_parameters.ul_addr +=
		//VKB		rx_buffer_definition->rx_pdc_parameters.ul_size;

		/* If the end of the buffer has been reached, wrap back to the start. */
		//VKBif (rx_buffer_definition->rx_pdc_parameters.ul_addr >=
		//VKB		rx_buffer_definition->past_rx_buffer_end_address)
		//VKB{
		//VKB	rx_buffer_definition->rx_pdc_parameters.ul_addr =
		//VKB			rx_buffer_definition->rx_buffer_start_address;
		//VKB}

		/* Reset the Rx DMA to receive data into whatever free space remains in
		the Rx buffer. */
		configure_rx_dma(usart_index, data_added);

		if (rx_buffer_definition->rx_event_semaphore != NULL) {
			/* Notify that new data is available. */
			xSemaphoreGiveFromISR(
					rx_buffer_definition->rx_event_semaphore,
					&higher_priority_task_woken);
		}
	}

	if ((usart_status & US_IER_TIMEOUT) != 0UL) {
		/* More characters have been placed into the Rx buffer.

		Restart the timeout after more data has been received. */
		usart_start_rx_timeout(all_usart_definitions[usart_index].peripheral_base_address);

		if (rx_buffer_definition->rx_event_semaphore != NULL) {
			/* Notify that new data is available. */
			xSemaphoreGiveFromISR(
					rx_buffer_definition->rx_event_semaphore,
					&higher_priority_task_woken);
		}
	}

	if ((usart_status & SR_ERROR_INTERRUPTS) != 0) {
		/* An error occurred in either a transmission or reception.  Abort, and
		ensure the peripheral access mutex is made available to tasks. */
		usart_reset_status(
				all_usart_definitions[usart_index].peripheral_base_address);
		if (tx_dma_control[usart_index].peripheral_access_mutex != NULL) {
			xSemaphoreGiveFromISR(
					tx_dma_control[usart_index].peripheral_access_mutex,
					&higher_priority_task_woken);
		}
	}

	/* If giving a semaphore caused a task to unblock, and the unblocked task
	has a priority equal to or higher than the currently running task (the task
	this ISR interrupted), then higher_priority_task_woken will have
	automatically been set to pdTRUE within the semaphore function.
	portEND_SWITCHING_ISR() will then ensure that this ISR returns directly to
	the higher priority unblocked task. */
	portEND_SWITCHING_ISR(higher_priority_task_woken);
}

/*
 * Individual interrupt handlers follow from here.  Each individual interrupt
 * handler calls the common interrupt handler.
 */
#ifdef USART

void USART_Handler(void)
{
	local_usart_handler(0);
}

#endif /* USART */

#ifdef USART0

void USART0_Handler(void)
{
	local_usart_handler(0);
}

#endif /* USART0 */

#ifdef USART1

void USART1_Handler(void)
{
	local_usart_handler(1);
}

#endif /* USART1 */

/*#ifdef USART2

void USART2_Handler(void)
{
	local_usart_handler(2);
}

#endif*/ /* USART2 */

#ifdef USART3

void USART3_Handler(void)
{
	local_usart_handler(3);
}

#endif /* USART3 */

#ifdef USART4

void USART4_Handler(void)
{
	local_usart_handler(4);
}

#endif /* USART4 */

#ifdef USART5

void USART5_Handler(void)
{
	local_usart_handler(5);
}

#endif /* USART5 */

#ifdef USART6

void USART6_Handler(void)
{
	local_usart_handler(6);
}

#endif /* USART6 */
