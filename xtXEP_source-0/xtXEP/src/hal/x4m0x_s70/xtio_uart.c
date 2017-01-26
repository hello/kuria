
#include "xtio_uart.h"
#include "xep_hal.h"
#include "freertos_usart_serial.h"
#include "xtmemory.h"
#include "ioport.h"
#include "FreeRTOS.h"
#include "semphr.h"


#define USART_SERIAL                 USART0
#define USART_SERIAL_ID              ID_USART0  //USART0 for sam4l

#define SERIAL_RX_BUFFER_SIZE		(3000)
#define UART_SERIAL_BAUDRATE        115200 // MAC vs WIN // 921600

uint8_t * receive_buffer;
uint32_t buffer_counter = 0;

int xtio_uart_init(void)
{
    int32_t status = XTIO_SUCCESS;
    receive_buffer = (uint8_t *)xtmemory_malloc_default(SERIAL_RX_BUFFER_SIZE);

	sam_usart_opt_t usart_settings = {
		UART_SERIAL_BAUDRATE,
		US_MR_CHRL_8_BIT,
		US_MR_PAR_NO,
		US_MR_NBSTOP_1_BIT,
		US_MR_CHMODE_NORMAL
		//##KIH 0 /* Only used in IrDA mode. */
		}; /*_RB_ TODO This is not SAM specific, not a good thing. */

	//## usart_settings.baudrate = resetstore_get_baudrate();

	/* Initialise the USART interface. */
    sysclk_enable_peripheral_clock(USART_SERIAL_ID);
    usart_init_rs232(USART_SERIAL, &usart_settings, sysclk_get_main_hz()/2);
    usart_enable_tx(USART_SERIAL);
    usart_enable_rx(USART_SERIAL);

	usart_enable_interrupt(USART_SERIAL, US_IER_RXRDY);
	NVIC_EnableIRQ(USART0_IRQn);

    return status;
}

int xtio_uart_send(uint8_t * buffer, uint32_t length, int time_out_definition, SemaphoreHandle_t uart_tx_notification_semaphore)
{
    int status = XTIO_SUCCESS;
    uint32_t returned_status;

    for (uint32_t i = 0; i < length; i++)
    {
        returned_status = usart_putchar(USART_SERIAL, buffer[i]);
		if (returned_status != 0)
		{
			status = XT_ERROR;
			break;
		}
    }

    return status;
}

int xtio_uart_receive(uint8_t * buffer, uint32_t * length) //##, int time_out_definition, SemaphoreHandle_t uart_tx_notification_semaphore);
{
    int status = XTIO_SUCCESS;

    NVIC_DisableIRQ(USART0_IRQn);
    if ((buffer_counter > 0) && (*length > buffer_counter))
    {
        *length = buffer_counter;
        for (uint32_t i = 0; i < buffer_counter; i++)
        {
            buffer[i] = receive_buffer[i];
        }
        buffer_counter = 0;
    }
    else
    {
        // Error - buffer not big enough
        buffer_counter = 0;
        *length = buffer_counter;
        status = XT_ERROR;
    }
	NVIC_EnableIRQ(USART0_IRQn);

	return XTIO_SUCCESS;
}


void USART0_Handler(void)
{
    uint32_t dw_status = usart_get_status(USART_SERIAL);

    if (dw_status & US_CSR_RXRDY) {
        uint32_t received_byte;
        usart_getchar(USART_SERIAL, &received_byte);
        receive_buffer[buffer_counter] = (uint8_t)(received_byte & 0xFF);
        buffer_counter++;
    }
}
