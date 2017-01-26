/**
 * @file
 *
 * @brief Implement USB serial communication
 *
 * Using FreeRTOS
 */


#include <stdbool.h>
#include "cycle_counter.h"
#include "xep_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "protocol.h"
// From module: USB CDC Protocol
#include <usb_protocol_cdc.h>
#include "xtio_serial_com.h"
#include "xtio_uart.h"
#include "task_monitor.h"

// From module: USB Device CDC (Single Interface Device)
#include <udi_cdc.h>

// From module: USB Device Stack Core (Common API)
#include <udc.h>
#include <udd.h>

/// @todo Check size of UART_RX_READ_BUFFER_SIZE buffer
#define UART_RX_READ_BUFFER_SIZE 400

static xtProtocol * xt_protocol_local;

#define TASK_USB_RX_PRIORITY  (tskIDLE_PRIORITY + 3)
#define TASK_USB_RX_STACK_SIZE (2048)
#define TASK_USB_TX_PRIORITY  (tskIDLE_PRIORITY + 4)
#define TASK_USB_TX_STACK_SIZE (2048)

#define TASK_UART_RX_PRIORITY  (tskIDLE_PRIORITY + 3)
#define TASK_UART_RX_STACK_SIZE (2048)
#define TASK_UART_TX_PRIORITY  (tskIDLE_PRIORITY + 4)
#define TASK_UART_TX_STACK_SIZE (2048)

typedef struct {
        uint8_t * buf;
        uint32_t length;
} com_tx_item_t;


static volatile bool usbEnabled = false;
static volatile bool usbIdle = true;
static TaskHandle_t pTaskUsbRx;
static TaskHandle_t pTaskUsbTx;
static void _usb_task_rx(void *pvParameters);
static void _usb_task_tx(void *pvParameters);
void task_usb_deinit(void);

static volatile bool uartEnabled = false;
static volatile bool uartIdle = true;
static TaskHandle_t pTaskUartRx;
static TaskHandle_t pTaskUartTx;
static void _uart_task_rx(void *pvParameters);
static void _uart_task_tx(void *pvParameters);


QueueHandle_t com_tx_queue;


// Check speed functions
//

// Set baudrate
//

bool xtio_usb_available(void)
{
    bool usbPresent = true;
    int pin_level = 0;
    int status;

    /*
     @todo Check if we need so long time before reporting USB available
     OJE: This was probably due to some hw issue on the old modules.
     */
    for (int i = 0; i < 100; i++)
    {
        status = xtio_get_level(XTIO_USB_VBUS, &pin_level);
        if (XT_SUCCESS != status || 0 == pin_level)
        {
            usbPresent = false;
            break;
        }
        cpu_delay_ms(10, 32876);
    }
    return usbPresent;
}

int xtio_serial_com_init(xtProtocol * xt_protocol)
{
    int status = XT_SUCCESS;

    if (xtio_usb_available())
    {
        // Init USB
        status = xtio_task_usb_init(xt_protocol);
    }
    else
    {
        // Init UART
        status = xtio_task_uart_init(xt_protocol);
    }

    return status;
}

int xtio_host_send(uint8_t * buffer, uint32_t length)
{
    com_tx_item_t tx_item;
    tx_item.buf = buffer;
    tx_item.length = length;

    if (pdTRUE == xQueueSend(com_tx_queue, &tx_item, 100 / portTICK_PERIOD_MS))
    {
        return XT_SUCCESS;
    }
    else
    {
        return XT_ERROR;
    }
}

int xtio_task_usb_init(xtProtocol * xt_protocol)
{
    //##tx_buf_index = 0;
    xt_protocol_local = xt_protocol;

    usbEnabled = false;
    usbIdle = false;

    // Setup USB stack
    udc_start();

    // Queue of single tx item. Buffering messages is done prior to this stage, using the dispatch'er and memory pool mechanisms.
    com_tx_queue = xQueueCreate(1, sizeof(com_tx_item_t));

    xTaskCreate(_usb_task_rx, (const char * const) "tskUsbRx", TASK_USB_RX_STACK_SIZE, NULL, TASK_USB_RX_PRIORITY, &pTaskUsbRx);
    xTaskCreate(_usb_task_tx, (const char * const) "tskUsbTx", TASK_USB_TX_STACK_SIZE, NULL, TASK_USB_TX_PRIORITY, &pTaskUsbTx);

    return XT_SUCCESS;
}

// This function is called from USB driver (ASF)
bool task_usb_enable(void)
{
    usbEnabled = true;
    return true;
}

// This function is called from USB driver (ASF)
void task_usb_disable(void)
{
    usbEnabled = false;
}

void task_usb_deinit(void)
{
    // Signal that we intend to shutdown the USB stack and wait until
    // ongoing read/write operations are completed before shutdown.
    usbEnabled = false;
    while (!usbIdle)
    {
        vTaskDelay(10UL / portTICK_PERIOD_MS);
    }
    udc_stop();
}

static void _usb_task_tx(void *pvParameters)
{
    (void)pvParameters;
    com_tx_item_t tx_item;
    uint32_t status = XT_SUCCESS;

    monitor_task_t * monitor_task_handle;
    status = monitor_task_register(&monitor_task_handle, 1000);


    while (true)
    {
        monitor_task_alive(monitor_task_handle);
        if (!usbEnabled)
        {
            usbIdle = true;
            vTaskDelay(1UL / portTICK_PERIOD_MS);
            continue;
        }
        usbIdle = false;

        if (udi_cdc_is_tx_ready() && (xQueueGenericReceive(com_tx_queue, &tx_item, 500 / portTICK_PERIOD_MS, pdTRUE)))
        {
            // Item fetched from queue, but not removed from the queue yet. Continue blocking next send.

            // Data received from the queue. Ensure all bytes are written.
            uint32_t bytesLeft = tx_item.length;
            uint8_t *dataPtr = tx_item.buf;
            while (bytesLeft && usbEnabled)
            {
                uint32_t remaining = udi_cdc_write_buf(dataPtr, bytesLeft);
                if (remaining > bytesLeft)
                    remaining = bytesLeft;
                const uint32_t bytesWritten = bytesLeft - remaining;
                bytesLeft -= bytesWritten;
                dataPtr += bytesWritten;
            }

            xQueueReceive(com_tx_queue, &tx_item, 0); // Now remove from queue.
        }
    }
}

static void _usb_task_rx(void *pvParameters)
{
    (void)pvParameters;
    uint8_t rxbuff[UART_RX_READ_BUFFER_SIZE];
    uint32_t status = XT_SUCCESS;

    monitor_task_t * monitor_task_handle;
    status = monitor_task_register(&monitor_task_handle, 1000);

    while (true)
    {
        monitor_task_alive(monitor_task_handle);
        if (!usbEnabled)
        {
            usbIdle = true;
            vTaskDelay(1UL / portTICK_PERIOD_MS);
            continue;
        }
        usbIdle = false;

        uint32_t bytesAvailable = udi_cdc_get_nb_received_data();
        if (bytesAvailable > 0)
        {
            if (bytesAvailable > sizeof(rxbuff))
                bytesAvailable = sizeof(rxbuff);
            uint32_t remaining = udi_cdc_read_buf(rxbuff, bytesAvailable);
            if (remaining > bytesAvailable)
                remaining = bytesAvailable;
            const uint32_t bytesRead = bytesAvailable - remaining;
            // Note that partial reads are perfectly fine here as remaining
            // bytes will be read in the next iteration
            parseData(xt_protocol_local, rxbuff, bytesRead); // Parse received data.
        }
        vTaskDelay(10UL / portTICK_PERIOD_MS);
    }
}

////////////////////////////////////////////////////////////////////////////////
// UART part
//

int xtio_task_uart_init(xtProtocol * xt_protocol)
{
    int status = XT_SUCCESS;

    //##tx_buf_index = 0;
    xt_protocol_local = xt_protocol;

    uartEnabled = false;
    uartIdle = false;

    xtio_uart_init();

    // Queue of single tx item. Buffering messages is done prior to this stage, using the dispatch'er and memory pool mechanisms.
    com_tx_queue = xQueueCreate(1, sizeof(com_tx_item_t));

    xTaskCreate(_uart_task_rx, (const char * const) "tskUartRx", TASK_UART_RX_STACK_SIZE, NULL, TASK_UART_RX_PRIORITY, &pTaskUartRx);
    xTaskCreate(_uart_task_tx, (const char * const) "tskUartTx", TASK_UART_TX_STACK_SIZE, NULL, TASK_UART_TX_PRIORITY, &pTaskUartTx);

    uartEnabled = true;

    return status;
}

static void _uart_task_tx(void *pvParameters)
{
    (void)pvParameters;
    com_tx_item_t tx_item;
    int status = XT_SUCCESS;
	SemaphoreHandle_t uart_tx_notification_semaphore;
	const uint32_t time_out_definition = (100UL / portTICK_PERIOD_MS);

    monitor_task_t * monitor_task_handle;
    status = monitor_task_register(&monitor_task_handle, 1000);

	// Create the semaphore to be used to get notified of end of transmissions.
	vSemaphoreCreateBinary(uart_tx_notification_semaphore);
	configASSERT(uart_tx_notification_semaphore);


    while (true)
    {
        monitor_task_alive(monitor_task_handle);
        if (!uartEnabled)
        {
            uartIdle = true;
            vTaskDelay(1UL / portTICK_PERIOD_MS);
            continue;
        }
        uartIdle = false;

        if (/*udi_cdc_is_tx_ready() && */(xQueueGenericReceive(com_tx_queue, &tx_item, 500 / portTICK_PERIOD_MS, pdTRUE)))
        {
            // Item fetched from queue, but not removed from the queue yet. Continue blocking next send.
            status = xtio_uart_send(tx_item.buf, tx_item.length, time_out_definition, uart_tx_notification_semaphore);
    		if (status != XTIO_SUCCESS)
    		{
    			// TODO: Some handling
    		}
            xQueueReceive(com_tx_queue, &tx_item, 0); // Now remove from queue.

    		if (pdTRUE != xSemaphoreTake(uart_tx_notification_semaphore, time_out_definition * 2))
    		{
    			// TODO: Who knows...
    		}
        }
    }
}

static void _uart_task_rx(void *pvParameters)
{
    (void)pvParameters;
    int status = XTIO_SUCCESS;
    uint8_t rxbuff[UART_RX_READ_BUFFER_SIZE];
	uint32_t length = 0;

    monitor_task_t * monitor_task_handle;
    status = monitor_task_register(&monitor_task_handle, 1000);

    while (true)
    {
        monitor_task_alive(monitor_task_handle);
        if (!uartEnabled)
        {
            uartIdle = true;
            vTaskDelay(1UL / portTICK_PERIOD_MS);
            continue;
        }
        uartIdle = false;

        length = UART_RX_READ_BUFFER_SIZE;
        status = xtio_uart_receive(rxbuff, &length);
        if (length > 0)
        {
            uint32_t bytesRead = length;
            // Note that partial reads are perfectly fine here as remaining
            // bytes will be read in the next iteration
            parseData(xt_protocol_local, rxbuff, bytesRead); // Parse received data.
        }
/*
        uint32_t bytesAvailable = udi_cdc_get_nb_received_data();
        if (bytesAvailable > 0)
        {
            if (bytesAvailable > sizeof(rxbuff))
                bytesAvailable = sizeof(rxbuff);
            uint32_t remaining = udi_cdc_read_buf(rxbuff, bytesAvailable);
            if (remaining > bytesAvailable)
                remaining = bytesAvailable;
            const uint32_t bytesRead = bytesAvailable - remaining;
            // Note that partial reads are perfectly fine here as remaining
            // bytes will be read in the next iteration
            parseData(xt_protocol_local, rxbuff, bytesRead); // Parse received data.
        }
		*/
        vTaskDelay(10UL / portTICK_PERIOD_MS);
    }
}
