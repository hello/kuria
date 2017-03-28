#include <stdio.h>
#include <stdint.h>
#include "radar_task.h"
#include "x4driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include "semphr.h"
#include "timers.h"
#include "spidriver.h"
#include "gpio_hal.h"
#include "radar_data_format.h"
#include <string.h>

/* Defines */
#if 1
#define DISP printf
#else
#define DISP(...)
#endif

#define TASK_RADAR_STACK_SIZE            (1500)
#define TASK_RADAR_PRIORITY        (tskIDLE_PRIORITY + 7)

#define XEP_NOTIFY_RADAR_DATAREADY		0x0001
#define XEP_NOTIFY_RADAR_TRIGGER_SWEEP	0x0002
#define XEP_NOTIFY_X4DRIVER_ACTION		0x0004
#define XEP_NOTIFY_TASK_END             0x0010


/* Local Variables */
static X4Driver_t* x4driver;
static TaskHandle_t h_task_radar = NULL;
static bool en_intr = true;
static SemaphoreHandle_t xRadarSem;

/* Static function declarations */
static void x4driver_task(void* pvParameters);
static uint32_t read_and_send_radar_frame(X4Driver_t* x4driver);
void x4driver_timer_sweep_timeout(TimerHandle_t pxTimer) ;
void x4driver_timer_action_timeout(TimerHandle_t pxTimer) ;
uint32_t x4driver_timer_set_timer_timeout_frequency(void* timer , uint32_t frequency) ;
void x4driver_notify_data_ready(void* user_reference);
void x4driver_interrupt_notify_data_ready(void) ;
void x4driver_enable_ISR(void* user_reference, uint32_t enable) ;
uint32_t x4driver_trigger_sweep_pin(void* user_reference);
uint32_t x4driver_callback_pin_set_enable(void* user_reference, uint8_t value);
void x4driver_interrupt_data_ready(void) ;
static uint32_t x4driver_callback_take_sem(void * sem,uint32_t timeout);
static void x4driver_callback_give_sem(void * sem);

/* Global Variables */
extern QueueHandle_t radar_data_queue; 


/* Function definitions */

/* RADAR TASK INIT */
uint32_t radar_task_init(void){

    int status;

    /* Init Hal */

    // Init SPI
    spi_init();

    // Init GPIO
    gpio_init();

    xRadarSem = xSemaphoreCreateBinary();
    if(xRadarSem == NULL ){
        printf("Could not create semaphore \n");
        return -1;
    }

    // X4Driver calbacks
    X4DriverCallbacks_t x4driver_callbacks;
    x4driver_callbacks.pin_set_enable = x4driver_callback_pin_set_enable;   // X4 ENABLE pin
    x4driver_callbacks.spi_read = spi_read;               // SPI read method
    x4driver_callbacks.spi_write = spi_write;             // SPI write method
    x4driver_callbacks.spi_write_read = spi_write_read;   // SPI write and read method
#if USE_WIRING_PI
    x4driver_callbacks.wait_us = delayMicroseconds;                 // Delay method
#endif
    x4driver_callbacks.notify_data_ready = x4driver_notify_data_ready;      // Notification when radar data is ready to read
    x4driver_callbacks.trigger_sweep = x4driver_trigger_sweep_pin;          // Method to set X4 sweep trigger pin
    x4driver_callbacks.enable_data_ready_isr = x4driver_enable_ISR;         // Control data ready notification ISR

    // Initialize X4driver lock mechanism
    X4DriverLock_t lock;
    lock.object = (void*)xSemaphoreCreateRecursiveMutex();
    if(lock.object == NULL){ 
        printf(" create rec mutex fail\n");
        return -1;
    }
    lock.lock = x4driver_callback_take_sem;
    lock.unlock = x4driver_callback_give_sem;

    // X4Driver timer for generating sweep FPS on MCU. Not used when sweep FPS is generated on X4.
    uint32_t timer_id_sweep = 2;
    X4DriverTimer_t timer_sweep;
    timer_sweep.object = xTimerCreate("X4Driver_sweep_timer", 1000 / portTICK_PERIOD_MS, pdTRUE, (void*)timer_id_sweep, x4driver_timer_sweep_timeout);
    timer_sweep.configure = x4driver_timer_set_timer_timeout_frequency;

    // X4Driver timer used for driver action timeout.
    uint32_t timer_id_action = 3;
    X4DriverTimer_t timer_action;
    timer_action.object = xTimerCreate("X4Driver_action_timer", 1000 / portTICK_PERIOD_MS, pdTRUE, (void*)timer_id_action, x4driver_timer_action_timeout);
    timer_action.configure = x4driver_timer_set_timer_timeout_frequency;

    // Allocate memory for X4driver instance
    void* x4driver_instance_memory = pvPortMalloc(x4driver_get_instance_size());
    if(!x4driver_instance_memory){
        printf("malloc failed\n");
        return -1;
    }

    // Set X4driver instance variable to default values, initialize callbacks
    x4driver_create(&x4driver, x4driver_instance_memory, &x4driver_callbacks,&lock,&timer_sweep,&timer_action,NULL);

    // Create Radar task to read data from X4
    xTaskCreate(radar_task, (const char* const) "Radar", TASK_RADAR_STACK_SIZE, \
            NULL , TASK_RADAR_PRIORITY, &h_task_radar);

    // Enable X4 chip
    status = x4driver_set_enable(x4driver, 1);
    for(uint32_t i =0; i < 2000; i++) {
        //wait for it to be stable
        __asm__ volatile ("nop");		
    }
    printf("\n"); 

#if DUMP_SPI
    // Dump SPI register, for testing purposes only
    dump_spi_reg();
#endif

    // Initialize X4driver, update and verify firmware, set defaults
    status =  x4driver_init(x4driver);
    if(status !=  XEP_ERROR_X4DRIVER_OK) {
        printf ("Error initializing x4driver %d \n", status);
        return status;
    }
    else
        printf("X4Driver init success\n");


    /* Update X4 configurations for application */
    //
    // TODO - This values will have to be read from a file
    // Configure the radar chip as needed
    x4driver_set_dac_min(x4driver, 800);
    x4driver_set_dac_max(x4driver, 1254);
    x4driver_set_iterations(x4driver, 64);
    x4driver_set_pulses_per_step(x4driver, 6);
    x4driver_set_downconversion(x4driver, 1);
    //  x4driver_set_frame_area_offset(x4driver, 0.6);
    //  x4driver_set_frame_area(x4driver, 0.5, 9.9);
    x4driver_set_fps(x4driver, 10);

    // Verify X4 configurations
    status = x4driver_check_configuration(x4driver);
    if( status != XEP_ERROR_X4DRIVER_OK) {
        printf(" check config fail %d \n", status);
        return status;
    }

    // Set sweep trigger control
    if( x4driver_set_sweep_trigger_control(x4driver, SWEEP_TRIGGER_X4) ) {
        printf(" Set sweep trigger control fail\n");
        return status;
    }

    return status;

}

/* RADAR TASK */
void radar_task(void* pvParameters){

    printf("X4 Test start...\n");

    while(1) {
#if 0
        uint32_t notify_value = 0;
        // poll x4 for data
        //
        xTaskNotifyWait( 0x00, /* Dont clear any notification bits on entry */
                0xffffffff, /* Reset the notification value to 0 on exit. */
                &notify_value, /*Notified value pass out. */
#if 1 
                portMAX_DELAY 
#else
                500 / portTICK_PERIOD_MS  /* Block indefinitely. */
#endif
                );

        if (notify_value & XEP_NOTIFY_RADAR_DATAREADY) {

            printf("Radar Data Ready\n");

            if(x4driver->trigger_mode != SWEEP_TRIGGER_MANUAL) {
                printf("Read and send\n"); 
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
            break;
        }
        else{
            printf ("n");
        }
        //x4driver_interrupt_notify_data_ready();
        vTaskDelay(1);
#else
        if( xSemaphoreTake(xRadarSem, portMAX_DELAY) ){
            DISP("Read and send\n"); 
            read_and_send_radar_frame(x4driver);
        }
#endif

    } // Task forever loop

    printf("Ending X4 Test...\n");

}

void radar_task_end (void) {

    // Disable X4
    x4driver_set_enable(x4driver, 0);

    // Close spi
    spi_close ();
}

static int32_t radar_data_frame_prepare( radar_frame_packet** packet, uint32_t data_count ){

    *packet = (radar_frame_packet*) pvPortMalloc( sizeof(radar_frame_packet) );

    if( !(*packet) ) {
        printf( "Error creating radar packet \n");
        return -1;
    }

    memset( *packet, 0, sizeof( radar_frame_packet) );

    (*packet)->num_of_bins = data_count;

    (*packet)->fdata = (float32_t*) pvPortMalloc( sizeof(float32_t) * data_count );
    if( (*packet)->fdata == NULL ) {
        printf( "fdata malloc error\n" );
        return -1;
    }
    return 0;

}

int32_t radar_data_frame_free( radar_frame_packet* packet ) {
    if( packet->fdata ) {
        vPortFree(packet->fdata);
    }

    if( packet->sig_i ) {
        vPortFree( packet->sig_i );
    }

    if( packet->sig_q ) {
        vPortFree( packet->sig_q );
    }

    vPortFree( packet );

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
    radar_frame_packet* radar_packet = NULL;

    if( radar_data_frame_prepare(&radar_packet, fdata_count ) ) {
        printf(" error creating radar data frame \n");
        return -1;
    }

    // Read radar data into dispatch memory.
    status = x4driver_read_frame_normalized(x4driver, &radar_packet->frame_counter,radar_packet->fdata, radar_packet->num_of_bins);

    if (status != XEP_ERROR_X4DRIVER_OK) {
        radar_data_frame_free( radar_packet );
        return status;
    }
    else {
        //        printf("Frame read completed\n");
    }
#if 1 
    // send radar data to file task
    //
    if(radar_data_queue ){
        if( xQueueSend( radar_data_queue, &radar_packet, (TickType_t ) 100 ) != pdPASS ) {
            printf( "Failed to send msg to queue \n");
            radar_data_frame_free( radar_packet );
            return -1;
        }
    }
#else
    radar_data_frame_free( radar_packet);
#endif
    return status;
}

void dump_spi_reg(void){
    for(uint32_t i=ADDR_SPI_FORCE_ZERO_R; i<= ADDR_SPI_SPI_CONFIG_WE; i++){
        uint8_t value;
     //   if(i== ADDR_SPI_RADAR_DATA_SPI_RE) continue;
        x4driver_get_spi_register(x4driver,i ,&value );
        printf(" %d: %x\n", i, value);
    }

}

/************************ Callbacks ******************************************/

/* TIMER CALLBACKS */
void x4driver_timer_sweep_timeout(TimerHandle_t pxTimer) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(h_task_radar, XEP_NOTIFY_RADAR_TRIGGER_SWEEP, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );	
}


void x4driver_timer_action_timeout(TimerHandle_t pxTimer) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(h_task_radar, XEP_NOTIFY_X4DRIVER_ACTION, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
}

uint32_t x4driver_timer_set_timer_timeout_frequency(void* timer , uint32_t frequency) {

    uint32_t status = XEP_ERROR_X4DRIVER_OK;
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
    return status;
}

/* X4driver callbacks */
void x4driver_notify_data_ready(void* user_reference){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(h_task_radar, XEP_NOTIFY_RADAR_DATAREADY, eSetBits, 
            &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken ); 

}

void x4driver_interrupt_notify_data_ready(void) {
    static bool first_intr = true;
    // TODO ignore first interrupt since it writes junk data into file
    if (first_intr) {
        first_intr = false;
        return;
    }

    if( xSemaphoreGive(xRadarSem ) != pdTRUE ) {
        printf("sem give fail \n");
    }
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

    x4driver_enable_ISR(NULL,1);
    return XEP_ERROR_X4DRIVER_OK;
}

void x4driver_interrupt_data_ready(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(h_task_radar, XEP_NOTIFY_RADAR_DATAREADY, eSetBits, 
            &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); 
}

/* X4driver lock mechanism callbacks */
static uint32_t x4driver_callback_take_sem(void * sem,uint32_t timeout)
{
    return xSemaphoreTakeRecursive((SemaphoreHandle_t)sem, timeout);
}

static void x4driver_callback_give_sem(void * sem)
{
    xSemaphoreGiveRecursive((SemaphoreHandle_t)sem);
}

