#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include "spidriver.h"
#include "x4driver.h"

bool stop_x4_read = 0;
int spi_fd;
static X4Driver_t x4driver;

void sig_handler(int sig) {
    if(sig == SIGINT || sig == SIGABRT)  {
        stop_x4_read = 1;
    }
}

static void x4driver_callback_init(void) {
    
    // X4Driver callback methods.
	X4DriverCallbacks_t x4driver_callbacks;
    x4driver_callbacks.pin_set_enable = x4driver_callback_pin_set_enable;   // X4 ENABLE pin
    x4driver_callbacks.spi_read = spi_read;               // SPI read method
    x4driver_callbacks.spi_write = spi_write;             // SPI write method
    x4driver_callbacks.spi_write_read = spi_write_read;   // SPI write and read method
    x4driver_callbacks.wait_us = x4driver_callback_wait_us;                 // Delay method
    x4driver_callbacks.notify_data_ready = x4driver_notify_data_ready;      // Notification when radar data is ready to read
    x4driver_callbacks.trigger_sweep = x4driver_trigger_sweep_pin;          // Method to set X4 sweep trigger pin
    x4driver_callbacks.enable_data_ready_isr = x4driver_enable_ISR;         // Control data ready notification ISR
}

static void x4driver_timer_init(void){
}

static void x4driver_lock_init(void){
}

static void x4driver_task(void){


    printf("X4 Test start...\n");
    while(!stop_x4_read) {
        // poll x4 for data
        //
        sleep(1);
    }
    printf("Ending X4 Test...\n");

}

int main() {
    if(signal(SIGINT, sig_handler) == SIG_ERR){
        printf("can't catch SIGINT\n");
    }

    if(signal(SIGABRT, sig_handler) == SIG_ERR){
        printf("can't catch SIGABRT\n");
    }

    spi_init(&spi_fd);

    x4driver_callback_init();
    x4driver_timer_init();
    x4driver_lock_init();


    // Open file to save data
    // Initialize x4 module
    //
    x4driver_task();

    close(spi_fd);
    return 0;

}
