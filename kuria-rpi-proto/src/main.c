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
    printf("-\n");
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
    for(int i =0; i < 1000; i++) {
        //wait
        printf(".");
    }
    printf("\n");
    return status;

}

static void x4driver_task(void* pvParameters){

    int status =  x4driver_init(x4driver);
    if(status !=  XEP_ERROR_X4DRIVER_OK) {
        printf ("Error initializing x4driver %d \n", status);
    }
    else
        printf("X4Driver init success\n");
    x4driver_check_configuration(x4driver);
    x4driver_set_sweep_trigger_control(x4driver, SWEEP_TRIGGER_X4);
    if(status) {
        printf("Somethings not right: %d \n",status);
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
        } else if (notify_value & XEP_NOTIFY_RADAR_TRIGGER_SWEEP) {

            printf("start sweep\n");
        } else if (notify_value & XEP_NOTIFY_X4DRIVER_ACTION) {

            printf( "on action \n");
        } else if (notify_value == 0){ //Timeout
            printf ("n");
        }
        if(stop_x4_read) {
            break;
        }
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

    spi_init();
    printf("SPI init done \n");

    // init gpio
    gpio_init();
    printf("GPIO Init Done\n");

    if(x4driver_task_init()){
        spi_close(); 
        return -1;
    }

    // Open file to save data
    // Initialize x4 module
    //
    //x4driver_task(NULL);
    vTaskStartScheduler();

    spi_close();
    return 0;

}
