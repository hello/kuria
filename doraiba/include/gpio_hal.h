#ifndef __GPIO_HAL_H__
#define __GPIO_HAL_H__


#include "kuria_config.h"

#if USE_WIRING_PI 
#include <wiringPi.h>
#else

#endif

#define X4_ENABLE_PIN           3
#define X4_GPIO1                (4)
#define X4_GPIO2                21
#define X4_INTR_PIN             X4_GPIO1
#define X4_SWEEP_TRIGGER_PIN    X4_GPIO2

#if USE_WIRING_PI
#define INT_RISING_EDGE INT_EDGE_RISING
#endif


void gpio_init(void);
void set_pin_direction (int pin_number, int direction);
void pin_write (int pin_number, int value);
int pin_read (int pin_number);
int gpio_enable_isr (int pin, int edge_type, void (*isr_callback) (void) );

#endif
