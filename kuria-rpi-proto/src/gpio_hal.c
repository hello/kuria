#include "gpio_hal.h"
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


void gpio_init(void) {

    // init wiringpi
    if( wiringPiSetup() < 0 ){
        fprintf( stderr, "Unable to setup wiringPi: %s\n", strerror (errno)) ;
        return;
    }

    // Config enable pin
    set_pin_direction (X4_ENABLE_PIN,OUTPUT);

    // Set pin low
    pin_write (X4_ENABLE_PIN, LOW);

    // Config Sweep trigger pin
    set_pin_direction (X4_SWEEP_TRIGGER_PIN,OUTPUT);

    return;
}

void set_pin_direction (int pin_number, int direction) {
#if USE_WIRING_PI
    pinMode (pin_number, direction);
#endif
}

void pin_write (int pin_number, int value) {
#if USE_WIRING_PI
    digitalWrite (pin_number, value);
#endif
}

int pin_read (int pin_number) {
#if USE_WIRING_PI
    return digitalRead (pin_number);
#endif
}

int gpio_enable_isr (int pin, int edge_type, void (*isr_callback) (void) ) {
#if USE_WIRING_PI
    return wiringPiISR (pin, edge_type, isr_callback );
#endif
}
