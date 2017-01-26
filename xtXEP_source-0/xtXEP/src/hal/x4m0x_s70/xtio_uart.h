/**
 * @file
 *
 * @brief Local header file for xtio UART functions.
 */

#ifndef XTIO_UART_H
#define  XTIO_UART_H

#include "xep_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"

int xtio_uart_init(void);
int xtio_uart_send(uint8_t * buffer, uint32_t length, int time_out_definition, SemaphoreHandle_t uart_tx_notification_semaphore);
int xtio_uart_receive(uint8_t * buffer, uint32_t * length); //##, int time_out_definition, SemaphoreHandle_t uart_tx_notification_semaphore);

#endif //  XTIO_UART_H
