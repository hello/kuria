#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include "spidriver.h"
#include "x4driver.h"
#include <wiringPi.h>
#include "portmacro.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "queue.h"
#include <errno.h>
#include <string.h>
#include "memorypool.h"
#include "xep_dispatch.h"
#include "xep_dispatch_messages.h"


#define TASK_RADAR_STACK_SIZE            (1500)
#define TASK_RADAR_PRIORITY        (tskIDLE_PRIORITY + 6)

#define XEP_NOTIFY_RADAR_DATAREADY		0x0001
#define XEP_NOTIFY_RADAR_TRIGGER_SWEEP	0x0002
#define XEP_NOTIFY_X4DRIVER_ACTION		0x0004

#define mainCHECK_DELAY       ( ( TickType_t ) 5000 / portTICK_RATE_MS )

#define X4_ENABLE_PIN           3
#define X4_GPIO1                (4)
#define X4_GPIO2                21
#define X4_INTR_PIN             X4_GPIO1
#define X4_SWEEP_TRIGGER_PIN    X4_GPIO2

bool stop_x4_read = 0;
static X4Driver_t* x4driver;
static TaskHandle_t h_task_radar = NULL;
static X4Driver_t x4driver_instance;
static bool en_intr = true;
static unsigned long long uxQueueSendPassedCount = 0;

static void x4driver_task(void* pvParameters);
static uint32_t read_and_send_radar_frame(X4Driver_t* x4driver, XepDispatch_t* dispatch);

void sig_handler(int sig) {
    if(sig == SIGINT || sig == SIGABRT)  {
        stop_x4_read = 1;
    }
}

/*************************** FreeRTOS application hooks**************************** */
void vApplicationTickHook( void )
{
    static unsigned long ulTicksSinceLastDisplay = 0;
    static unsigned long ulCalled = 0;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* Called from every tick interrupt.  Have enough ticks passed to make it
    time to perform our health status check again? */
    ulTicksSinceLastDisplay++;
    if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
    {
    ulTicksSinceLastDisplay = 0;
    ulCalled++;
    //printf("AppTickHook %ld\r\n", ulCalled);
    }
}

void vApplicationIdleHook( void )
{
    /* The co-routines are executed in the idle task using the idle task hook. */
    /* vCoRoutineSchedule(); */ /* Comment this out if not using Co-routines. */

    struct timespec xTimeToSleep, xTimeSlept;
    if(stop_x4_read) {
        printf("Idle task end\n"); 
        fflush(stdout);
        vTaskEndScheduler();
    }
    /* Makes the process more agreeable when using the Posix simulator. */
    xTimeToSleep.tv_sec = 1;
    xTimeToSleep.tv_nsec = 0;
    nanosleep( &xTimeToSleep, &xTimeSlept );
}

void vMainQueueSendPassed( void )
{
	/* This is just an example implementation of the "queue send" trace hook. */
	uxQueueSendPassedCount++;
}

/************************ X4Driver callbacks ******************************************/

void x4driver_timer_sweep_timeout(TimerHandle_t pxTimer)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(h_task_radar, XEP_NOTIFY_RADAR_TRIGGER_SWEEP, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );	
}


void x4driver_timer_action_timeout(TimerHandle_t pxTimer)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(h_task_radar, XEP_NOTIFY_X4DRIVER_ACTION, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
}

uint32_t x4driver_timer_set_timer_timeout_frequency(void* timer , uint32_t frequency)
{

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

void x4driver_notify_data_ready(void* user_reference){
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(h_task_radar, XEP_NOTIFY_RADAR_DATAREADY, eSetBits, 
            &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken ); 

}

void x4driver_interrupt_notify_data_ready(void) {
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(h_task_radar, XEP_NOTIFY_RADAR_DATAREADY, eSetBits, 
            &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken ); 

}

void x4driver_enable_ISR(void* user_reference, uint32_t enable) {
    if(( enable == 1) && (en_intr ))  {
        // register interrupt callback for rising edge
        printf("Enabling ISR....\n");
        if( wiringPiISR (X4_INTR_PIN, INT_EDGE_RISING, &x4driver_interrupt_notify_data_ready) < 0 ) {
            printf("Unable to setup ISR \n");
        }
        en_intr = false;
        digitalWrite(X4_SWEEP_TRIGGER_PIN, LOW);
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
    digitalWrite(X4_SWEEP_TRIGGER_PIN, HIGH); 
    // Set IO2 to level low
    //
    digitalWrite(X4_SWEEP_TRIGGER_PIN, LOW); 
    return XEP_ERROR_X4DRIVER_OK;
}

uint32_t x4driver_callback_pin_set_enable(void* user_reference, uint8_t value){
	x4driver_enable_ISR(NULL,0);
    //set X4_ENABLE to value
    if( value == 0 ) digitalWrite(X4_ENABLE_PIN, LOW);
    else if( value == 1) digitalWrite(X4_ENABLE_PIN, HIGH);

    x4driver_enable_ISR(NULL,1);
	return XEP_ERROR_X4DRIVER_OK;
}

void x4driver_interrupt_data_ready(void) {
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(h_task_radar, XEP_NOTIFY_RADAR_DATAREADY, eSetBits, 
            &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); 
}

static uint32_t x4driver_callback_take_sem(void * sem,uint32_t timeout)
{
    return xSemaphoreTakeRecursive((SemaphoreHandle_t)sem, timeout);
}

static void x4driver_callback_give_sem(void * sem)
{
    xSemaphoreGiveRecursive((SemaphoreHandle_t)sem);
}

/*************************** Initializations *******************************/
static void gpio_init(void) {
    // init wiringpi
    //
    if( wiringPiSetup() < 0 ){
        fprintf( stderr, "Unable to setup wiringPi: %s\n", strerror (errno)) ;
        return;
    }

    // Config enable pin
    //
    pinMode(X4_ENABLE_PIN,OUTPUT);
    digitalWrite(X4_ENABLE_PIN, LOW);
    // Config ISR pin
    //
    //pinMode(X4_INTR_PIN,INPUT);

    // Config Sweep trigger pin
    //
    pinMode(X4_SWEEP_TRIGGER_PIN,OUTPUT);

    return;
}

static uint32_t x4driver_task_init(void){

    X4DriverCallbacks_t x4driver_callbacks;
    x4driver_callbacks.pin_set_enable = x4driver_callback_pin_set_enable;   // X4 ENABLE pin
    x4driver_callbacks.spi_read = spi_read;               // SPI read method
    x4driver_callbacks.spi_write = spi_write;             // SPI write method
    x4driver_callbacks.spi_write_read = spi_write_read;   // SPI write and read method
    x4driver_callbacks.wait_us = delayMicroseconds;                 // Delay method
    x4driver_callbacks.notify_data_ready = x4driver_notify_data_ready;      // Notification when radar data is ready to read
    x4driver_callbacks.trigger_sweep = x4driver_trigger_sweep_pin;          // Method to set X4 sweep trigger pin
    x4driver_callbacks.enable_data_ready_isr = x4driver_enable_ISR;         // Control data ready notification ISR

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

    void* x4driver_instance_memory = pvPortMalloc(x4driver_get_instance_size());
    if(!x4driver_instance_memory){
        printf("malloc failed\n");
        return -1;
    }

    x4driver_create(&x4driver, x4driver_instance_memory, &x4driver_callbacks,&lock,&timer_sweep,&timer_action,NULL);
   
    
    xTaskCreate(x4driver_task, (const char* const) "Radar", TASK_RADAR_STACK_SIZE, \
           NULL , TASK_RADAR_PRIORITY, &h_task_radar);

    int status = x4driver_set_enable(x4driver, 1);
    for(int i =0; i < 2000; i++) {
        //wait
        printf("");
    }
    printf("\n");
    return status;

}

static void x4driver_task(void* pvParameters){

    int status =  x4driver_init(x4driver);
    if(status !=  XEP_ERROR_X4DRIVER_OK) {
        printf ("Error initializing x4driver %d \n", status);
        goto x4task_fail;
    }
    else
        printf("X4Driver init success\n");
#if 0
    // Configure the radar chip as needed
    x4driver_set_dac_min(x4driver, 500);
    x4driver_set_dac_max(x4driver, 1500);
    x4driver_set_iterations(x4driver, 15);
    x4driver_set_pulses_per_step(x4driver, 10);
    x4driver_set_downconversion(x4driver, 1);
    x4driver_set_frame_area_offset(x4driver, 0.6);
    x4driver_set_frame_area(x4driver, 0.5, 4.0);
    x4driver_set_fps(x4driver, 20);
#endif
    x4driver_set_downconversion(x4driver, 1);
    status = x4driver_check_configuration(x4driver);
    if( status != XEP_ERROR_X4DRIVER_OK) {
        printf(" check config fail %d \n", status);
        goto x4task_fail;
    }
    if( x4driver_set_sweep_trigger_control(x4driver, SWEEP_TRIGGER_X4) ) {
        printf(" Set sweep trigger control fail\n");
        goto x4task_fail;
    }
   
    uint32_t notify_value;
    printf("X4 Test start...\n");
    while(1) {
        // poll x4 for data
        //
        xTaskNotifyWait( 0x00, /* Dont clear any notification bits on entry */
                         0xffffffff, /* Reset the notification value to 0 on exit. */
                         &notify_value, /*Notified value pass out. */
                         500 / portTICK_PERIOD_MS ); /* Block indefinitely. */

        if (notify_value & XEP_NOTIFY_RADAR_DATAREADY) {

            printf("Radar Data Ready\n");

            if(x4driver->trigger_mode != SWEEP_TRIGGER_MANUAL) {
                printf("Read and send\n"); 
                read_and_send_radar_frame(x4driver, NULL);
            }

        } else if (notify_value & XEP_NOTIFY_RADAR_TRIGGER_SWEEP) {

            printf("start sweep\n");
            x4driver_start_sweep(x4driver);
        } else if (notify_value & XEP_NOTIFY_X4DRIVER_ACTION) {

            printf( "on action \n");
            x4driver_on_action_event(x4driver);
        } else if (notify_value == 0){ //Timeout
            printf ("n");
        }
        if(stop_x4_read) {
            break;
        }
    }
    printf("Ending X4 Test...\n");

x4task_fail:
    status = x4driver_set_enable(x4driver, 0);
    for(int i =0; i < 2000; i++) {
        //wait
        printf("");
    }
    printf("\n");
    if( !stop_x4_read) {
        stop_x4_read = 1;
    }else {
        printf("x4driver task fail\n"); 
    }
    vTaskDelay(5);
    vTaskDelete( NULL );

}

static uint32_t read_and_send_radar_frame(X4Driver_t* x4driver, XepDispatch_t* dispatch) {

    uint32_t status;

    XepDispatchMessageContentRadardataFramePacket_t* frame_packet;
    MemoryBlock_t* memoryblock;

    uint32_t bin_count = 0;
    x4driver_get_frame_bin_count(x4driver, &bin_count);
    uint8_t down_convertion_enabled = 0;
    x4driver_get_downconvertion(x4driver, &down_convertion_enabled);
    
    printf("Down convertion enabled: %d\n Bin count: %d\n", down_convertion_enabled, bin_count);
    uint32_t fdata_count = bin_count;
    if(down_convertion_enabled == 1) {
        fdata_count = bin_count * 2;
    }

  /*  status = dispatch_message_radardata_prepare_frame(&frame_packet, &memoryblock, dispatch, fdata_count);
    if (status != XEP_ERROR_OK ) {
        return status;
    }*/
    uint32_t frame_counter;
    uint8_t* framedata = pvPortMalloc(x4driver->frame_read_size);
    if(!framedata) {
        printf("malloc failed\n");
        return -1;
    }
    printf("Prepare frame message done %d\n", x4driver->frame_read_size);
    // Read radar data into dispatch memory.
    status = x4driver_read_frame_normalized(x4driver, &frame_counter,(float32_t*) framedata, frame_packet->bin_count);
    printf("Frame read completed\n");
    vPortFree(framedata);
    if (status != XEP_ERROR_OK) {
 //       memorypool_free(memoryblock);
        return status;
    }
//    status = dispatch_send_message(NULL, dispatch, XEP_DISPATCH_MESSAGETAG_RADAR_DATA, memoryblock, frame_packet->common.message_size);

    return status;
}
int main() {
    if(signal(SIGINT, sig_handler) == SIG_ERR){
        printf("can't catch SIGINT\n");
    }

    if(signal(SIGABRT, sig_handler) == SIG_ERR){
        printf("can't catch SIGABRT\n");
    }

    spi_init();
    printf("SPI init done \n");

    usleep(100);
    // init gpio
    gpio_init();
    printf("GPIO Init Done\n");

    for(int i =0; i < 2000; i++) {
        //wait
        printf("");
    }
    if(x4driver_task_init()){
        spi_close(); 
        return -1;
    }

    usleep(100);
    // Open file to save data
    // Initialize x4 module
    //
    //x4driver_task(NULL);
    vTaskStartScheduler();
    printf("Exit from scheduler\n");
    spi_close();
    return 0;

}
