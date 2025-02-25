#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "radar_task.h"
#include "x4driver.h"
#include "kuria_config.h"
#include "dispatch_radar_frame.h"
#include "spidriver.h"
#include "gpio_hal.h"
#include "radar_data_format.h"
#include "hlo_notify.h"

/* Defines */
#if 0
#define DISP printf
#else
#define DISP(...)
#endif

#define DUMP_SPI 0
#define HLO_NOTIFY_MASK                 (0xFFFFFFFF)

#define TASK_RADAR_STACK_SIZE           (1500)
#define TASK_RADAR_PRIORITY             (tskIDLE_PRIORITY + 7)

#define XEP_NOTIFY_RADAR_DATAREADY		0x0001
#define XEP_NOTIFY_RADAR_TRIGGER_SWEEP	0x0002
#define XEP_NOTIFY_X4DRIVER_ACTION		0x0004
#define XEP_NOTIFY_TASK_END             0x0010


hlo_x4_config_t hlo_x4_config_default = {
    .dac_min = 800,
    .dac_max = 1254,
    .iterations = 32,
    .pps = 6,
    .downconversion_en = 1,
    .fps = 20,
    .tx_center_freq = TX_CENTER_FREQUENCY_KCC_8_748GHz
};

/* Local Variables */
static X4Driver_t* x4driver;
static bool en_intr = true;

hlo_notify_t radar_task_notify;

static pthread_mutex_t radar_task_mutex;


/* Static function declarations */
static uint32_t read_and_send_radar_frame(X4Driver_t* x4driver);
void dump_spi_reg(void);

// timer callbacks
void x4driver_timer_sweep_timeout(void* ctx) ;
void x4driver_timer_action_timeout(void* ctx) ;

uint32_t x4driver_timer_set_timer_timeout_frequency(void* timer , uint32_t frequency) ;
void x4driver_notify_data_ready(void* user_reference);
void x4driver_interrupt_notify_data_ready(void) ;
void x4driver_enable_ISR(void* user_reference, uint32_t enable) ;
uint32_t x4driver_trigger_sweep_pin(void* user_reference);
uint32_t x4driver_callback_pin_set_enable(void* user_reference, uint8_t value);
void x4driver_interrupt_data_ready(void) ;
static uint32_t x4driver_callback_take_sem(void * sem,uint32_t timeout);
static void x4driver_callback_give_sem(void * sem);
static int32_t radar_task_set_callbacks_timer( );
static int32_t radar_task_set_callbacks_lock ();
static int32_t radar_task_set_callbacks_driver ();
static int32_t hlo_x4_set_config (void); 

/* Global Variables */


/* Function definitions */

/* RADAR TASK INIT */
int32_t radar_task_init (void) {

    int status;

    /* Init Hal */

    // Init SPI
    spi_init();

    // Init GPIO
    gpio_init();

    // setup notify to signal task from ISR
    status = hlo_notify_init (&radar_task_notify);
    if (status) {
        printf (" error creating radar_task_notify: %d\n", status);
    }

    // initialize dispatcher to publish radar data
    if (dispatcher_init ()) {
        printf ("dispatcher init failed\n");
    }

    // X4Driver calbacks
    X4DriverCallbacks_t x4driver_callbacks;
    status = radar_task_set_callbacks_driver (&x4driver_callbacks);
    if (status) {
        printf ("error setting driver callbacks: %d\n", status);
        return status;
    }


    // Initialize X4driver lock mechanism
    X4DriverLock_t lock;
    status = radar_task_set_callbacks_lock (&lock);
    if (status) {
        printf ("error setting lock callbacks: %d\n", status);
        return status;
    }

    // X4Driver timer for generating sweep FPS on MCU. Not used when sweep FPS is generated on X4.
    uint32_t timer_id_sweep = 2;
    X4DriverTimer_t timer_sweep;

#if 0
    // TODO - incomplete
    // Use Linux timer
    timer_t sweep_timer_id;
    struct sigevent sev;

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.
        status = timer_create (CLOCK_THREAD_CPUTIME_ID,NULL, &sweep_timer_id);
    if (status) {
        printf ("Error creating sweep timer\n");
        return status;
    }

    timer_sweep.object = &sweep_timer_id;
#endif

    timer_sweep.configure = x4driver_timer_set_timer_timeout_frequency;

    // X4Driver timer used for driver action timeout.
    uint32_t timer_id_action = 3;
    X4DriverTimer_t timer_action;
    timer_action.configure = x4driver_timer_set_timer_timeout_frequency;

    // Allocate memory for X4driver instance
    void* x4driver_instance_memory = malloc(x4driver_get_instance_size());
    if(!x4driver_instance_memory){
        printf("malloc failed\n");
        return -1;
    }

    // Set X4driver instance variable to default values, initialize callbacks
    x4driver_create(&x4driver, x4driver_instance_memory, &x4driver_callbacks,&lock,&timer_sweep,&timer_action,NULL);

    // TODO add in new fw changes
    x4driver->frame_buffer_size = 157*32;
    x4driver->frame_buffer = malloc (x4driver->frame_buffer_size);
    assert (x4driver->frame_buffer);
    if ( ( ( (uint32_t) x4driver->frame_buffer) % 32) != 0) {
        printf ("alignment diff frame buffer\n");
        int alignment_diff = 32 - ( ( (uint32_t) x4driver->frame_buffer) % 32);
        // TODO umm, is this a memory leak
        x4driver->frame_buffer += alignment_diff;
        x4driver->frame_buffer_size -= alignment_diff;
    }
    x4driver->frame_buffer_size -= x4driver->frame_buffer_size % 32;
    printf ("frame buffer size: %d\n", x4driver->frame_buffer_size);

    // Enable X4 chip
    status = x4driver_set_enable(x4driver, 1);
    for(uint32_t i =0; i < 2000; i++) {
        //wait for it to be stable
        __asm__ volatile ("nop");		
    }
    printf("\n"); 


    // Initialize X4driver, update and verify firmware, set defaults
    status =  x4driver_init(x4driver);
    if(status !=  XEP_ERROR_X4DRIVER_OK) {
        printf ("Error initializing x4driver %d \n", status);
#if (DUMP_SPI==1)
        // Dump SPI register, for testing purposes only
        dump_spi_reg();
#endif
        return status;
    }
    else
        printf("X4Driver init success\n");

    return hlo_x4_set_config ();

}

static int32_t hlo_x4_set_config (void) {

    hlo_x4_config_t config = hlo_x4_config_default;

    int32_t status;

    /* Update X4 configurations for application */
    //
    status = hlo_x4_read_config_from_file (HLO_X4_CONFIG_FILE, &config);
    if (status) {
        printf ("Error reading config from file\n");
    }

    // Configure the radar chip as needed
    x4driver_set_dac_min(x4driver,              config.dac_min);
    x4driver_set_dac_max(x4driver,              config.dac_max);
    x4driver_set_iterations(x4driver,           config.iterations);
    x4driver_set_pulses_per_step(x4driver,      config.pps);
    x4driver_set_downconversion(x4driver,       config.downconversion_en);
    x4driver_set_fps(x4driver,                  config.fps);
    x4driver_set_tx_center_frequency(x4driver,  config.tx_center_freq);
    //  x4driver_set_frame_area_offset(x4driver, 0.6);
    //  x4driver_set_frame_area(x4driver, 0.5, 9.9);

    // Verify X4 configurations
    status = x4driver_check_configuration(x4driver);
    if( status != XEP_ERROR_X4DRIVER_OK) {
        printf(" check config fail %d \n", status);
        return status;
    }

    // Set sweep trigger control
    // By default let sweep trigger control done by X4
    if( x4driver_set_sweep_trigger_control(x4driver, SWEEP_TRIGGER_X4) ) {
        printf(" Set sweep trigger control fail\n");
        return status;
    }

    return status;
}

/* RADAR TASK */
void radar_task (void) {

    uint32_t notify_value = 0;

    printf("X4 Test start...\n");

    while(1) {

        hlo_notify_wait (&radar_task_notify, &notify_value, HLO_NOTIFY_MASK); 

        if (notify_value & XEP_NOTIFY_RADAR_DATAREADY) {


            if(x4driver->trigger_mode != SWEEP_TRIGGER_MANUAL) {
                DISP("Read and send\n"); 
                read_and_send_radar_frame(x4driver);
            }

        } else if (notify_value & XEP_NOTIFY_RADAR_TRIGGER_SWEEP) {

            printf("start sweep\n");
            x4driver_start_sweep(x4driver);
        } else if (notify_value & XEP_NOTIFY_X4DRIVER_ACTION) {

            printf( "on action \n");
            x4driver_on_action_event(x4driver);
        } else if (notify_value & XEP_NOTIFY_TASK_END) {
            printf("Ending radar task\n");
            radar_task_end ();
            exit (0);
        }

        notify_value = 0;

    } // Task forever loop

    printf("Ending X4 Test...\n");

}

void radar_task_end (void) {

    // close dispatcher
    dispatcher_close ();

    // Disable X4
    x4driver_set_enable(x4driver, 0);

    // Close spi
    spi_close ();
}

void radar_task_en_intr (void) {

    x4driver_enable_ISR(NULL,1);
}

static int32_t radar_data_frame_prepare( radar_frame_packet_t** packet, uint32_t data_count ){

    *packet = (radar_frame_packet_t*) malloc( sizeof(radar_frame_packet_t) );

    if( !(*packet) ) {
        printf( "Error creating radar packet \n");
        return -1;
    }

    memset( *packet, 0, sizeof( radar_frame_packet_t) );

    (*packet)->num_of_bins = data_count;

    (*packet)->fdata = (float32_t*) malloc( sizeof(float32_t) * data_count );

    if( (*packet)->fdata == NULL ) {
        printf( "fdata malloc error\n" );
        return -1;
    }
    return 0;

}

int32_t radar_data_frame_free( radar_frame_packet_t* packet, bool free_fdata ) {

    if (free_fdata) {
        if( packet->fdata ) {
            free(packet->fdata);
        }
    }
    if( packet->sig_i ) {
        free( packet->sig_i );
    }

    if( packet->sig_q ) {
        free( packet->sig_q );
    }

    free( packet );

    return 0;

}

static uint32_t read_and_send_radar_frame(X4Driver_t* x4driver) {
    int32_t status = XEP_ERROR_X4DRIVER_OK;

    // get bin count
    uint32_t bin_count = 0;
    x4driver_get_frame_bin_count(x4driver, &bin_count);

    // is downconvertion enabled
    uint8_t down_convertion_enabled = 0;
    x4driver_get_downconvertion(x4driver, &down_convertion_enabled);

    // calculate data count
    uint32_t fdata_count = bin_count;
    if(down_convertion_enabled == 1) {
        fdata_count = bin_count * 2;
    }

    // prepare radar data frame
    //
    radar_frame_packet_t* radar_packet = NULL;

    if( radar_data_frame_prepare(&radar_packet, fdata_count ) ) {
        printf(" error creating radar data frame \n");
        return -1;
    }

    radar_packet->content_id = down_convertion_enabled;
    // Read radar data into dispatch memory.
    status = x4driver_read_frame_normalized(x4driver, &radar_packet->frame_counter,radar_packet->fdata, radar_packet->num_of_bins);

    DISP("frame counter: %d\n", radar_packet->frame_counter);

    if (status != XEP_ERROR_X4DRIVER_OK) {
        printf ("error reading frame\n");
        radar_data_frame_free( radar_packet, true );
        return status;
    }
    else {
        //        printf("Frame read completed\n");
    }

    // publish radar data
    status = dispatch_radar_frame (radar_packet);
    if (status) {
        //        printf ("radar fram could not be dispatched\n");
    }

    radar_data_frame_free (radar_packet, true);

    return status;
}

void dump_spi_reg(void){
    printf("SPI\n");
    for(uint32_t i=ADDR_SPI_FORCE_ZERO_R; i<= ADDR_SPI_SPI_CONFIG_WE; i++){
        uint8_t value;
        //   if(i== ADDR_SPI_RADAR_DATA_SPI_RE) continue;
        x4driver_get_spi_register(x4driver,i ,&value );
        printf(" %x: %x\n", i, value);
    }

}

static int32_t radar_task_set_callbacks_timer (void ) {

    return 0;
}

static int32_t radar_task_set_callbacks_lock (X4DriverLock_t* lock) {

    int status = 0;

    printf(" Init pthread lock\n");

    // set mutex attribute to be recursive
    pthread_mutexattr_t attr;

    status = pthread_mutexattr_init (&attr);
    if (status) {
        printf ("mutex attr init fail: %d\n", status);
        return status;
    }

    status = pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
    if (status) {
        printf ("mutex attr set type fail: %d\n", status);
        return status;
    }

    // initialize recursive mutex
    status = pthread_mutex_init (&radar_task_mutex, &attr);
    if (status) {
        printf ("mutex init fail: %d\n", status);
        return status;
    }

    // update x4driver lock
    lock->object = (void*) &radar_task_mutex;


    if(lock->object == NULL){ 
        printf(" create rec mutex fail\n");
        return -1;
    }
    lock->lock = x4driver_callback_take_sem;
    lock->unlock = x4driver_callback_give_sem;

    return 0;
}

static int32_t radar_task_set_callbacks_driver (X4DriverCallbacks_t* x4driver_callbacks) {

    x4driver_callbacks->pin_set_enable = x4driver_callback_pin_set_enable;   // X4 ENABLE pin
    x4driver_callbacks->spi_read = spi_read;               // SPI read method
    x4driver_callbacks->spi_write = spi_write;             // SPI write method
    x4driver_callbacks->spi_write_read = spi_write_read;   // SPI write and read method
    x4driver_callbacks->wait_us = hlo_delay_us;                 // Delay method
    x4driver_callbacks->notify_data_ready = x4driver_notify_data_ready;      // Notification when radar data is ready to read
    x4driver_callbacks->trigger_sweep = x4driver_trigger_sweep_pin;          // Method to set X4 sweep trigger pin
    x4driver_callbacks->enable_data_ready_isr = x4driver_enable_ISR;         // Control data ready notification ISR
    return 0;
}

/************************ Callbacks ******************************************/


void x4driver_timer_sweep_timeout(void* ctx) {
    hlo_notify_send (&radar_task_notify, XEP_NOTIFY_RADAR_TRIGGER_SWEEP);
}

void x4driver_timer_action_timeout(void* ctx) {
    hlo_notify_send (&radar_task_notify, XEP_NOTIFY_X4DRIVER_ACTION);
}

uint32_t x4driver_timer_set_timer_timeout_frequency(void* timer , uint32_t frequency) {

    uint32_t status = XEP_ERROR_X4DRIVER_OK;
    // TODO remove when timer functionality is ported
#if 0

    uint32_t timer_ticks =  1000 / frequency / portTICK_PERIOD_MS;
    X4DriverTimer_t * x4drivertimer = (X4DriverTimer_t*)timer;
    x4drivertimer->configured_frequency = 1000.0 /timer_ticks;
    if(frequency == 0)
    {
        xTimerStop(x4drivertimer->object, 100 / portTICK_PERIOD_MS);
    }
    else
    {
        if (pdFAIL == xTimerChangePeriod(x4drivertimer->object, timer_ticks, 100 / portTICK_PERIOD_MS))
        {
            status = XEP_ERROR_X4DRIVER_TIMER_PERIOD_CHANGE_FAIL;
        }
    }
#endif
    return status;
}

/* X4driver callbacks */
void x4driver_notify_data_ready(void* user_reference){

    hlo_notify_send (&radar_task_notify, XEP_NOTIFY_RADAR_DATAREADY);

}

void x4driver_interrupt_notify_data_ready(void) {

    hlo_notify_send (&radar_task_notify, XEP_NOTIFY_RADAR_DATAREADY);
    DISP("INTR: \n");

}

void x4driver_enable_ISR(void* user_reference, uint32_t enable) {
    if(( enable == 1) && (en_intr ))  {
        // register interrupt callback for rising edge
        printf("Enabling ISR....\n");
        if( gpio_enable_isr (X4_INTR_PIN, INT_RISING_EDGE, &x4driver_interrupt_notify_data_ready) < 0 ) {
            printf("Unable to setup ISR \n");
        }
        en_intr = false;
        pin_write(X4_SWEEP_TRIGGER_PIN, LOW);
    }
    else {
        // unregister 
    }

    // Setup GPIO mode for IO2
    //
    //pinMode(X4_SWEEP_TRIGGER_PIN, OUTPUT);

    //Set level low
}

uint32_t x4driver_trigger_sweep_pin(void* user_reference){
    // Set IO2 to level high
    //
    pin_write(X4_SWEEP_TRIGGER_PIN, HIGH); 
    // Set IO2 to level low
    //
    pin_write(X4_SWEEP_TRIGGER_PIN, LOW); 
    return XEP_ERROR_X4DRIVER_OK;
}

uint32_t x4driver_callback_pin_set_enable(void* user_reference, uint8_t value){
    x4driver_enable_ISR(NULL,0);
    //set X4_ENABLE to value
    if( value == 0 ) pin_write(X4_ENABLE_PIN, LOW);
    else if( value == 1) pin_write(X4_ENABLE_PIN, HIGH);

    x4driver_enable_ISR(NULL,0);
    return XEP_ERROR_X4DRIVER_OK;
}

void x4driver_interrupt_data_ready(void) {

    hlo_notify_send (&radar_task_notify, XEP_NOTIFY_RADAR_DATAREADY);
}

/* X4driver lock mechanism callbacks */
static uint32_t x4driver_callback_take_sem(void * sem,uint32_t timeout)
{
    int status = pthread_mutex_lock ( (pthread_mutex_t*) sem );
    if (status) {
        printf ("pthread take lock fail\n");
        return KURIA_FALSE;
    }

    // return true if lock successful
    return KURIA_TRUE;
}

static void x4driver_callback_give_sem(void * sem)
{
    (void) pthread_mutex_unlock ( (pthread_mutex_t*) sem );
}

