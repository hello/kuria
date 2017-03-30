#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include "kuria_config.h"

#if USE_FREERTOS_TASKS

#include "portmacro.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#endif

#include <string.h>
#include "file_save.h"
#include "radar_task.h"

bool stop_x4_read = 0;

void sig_handler(int sig) {
    if(sig == SIGINT || sig == SIGABRT)  {
        stop_x4_read = 1;
    }
}

void end_application(void) {

    // Close files
    file_close();

    // End radar task
    radar_task_end ();

#if USE_FREERTOS_TASKS
    // End Scheduler
    vTaskEndScheduler();
#endif

}

#if USE_FREERTOS_TASKS

/*************************** FreeRTOS application hooks**************************** */
void vApplicationTickHook( void )
{
#if 0
    static unsigned long ulTicksSinceLastDisplay = 0;
    static unsigned long ulCalled = 0;

    /* Called from every tick interrupt.  Have enough ticks passed to make it
       time to perform our health status check again? */
    ulTicksSinceLastDisplay++;
    if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
    {
        ulTicksSinceLastDisplay = 0;
        ulCalled++;
        //printf("AppTickHook %ld\r\n", ulCalled);
    }
#endif
}

void vApplicationIdleHook( void )
{
    /* The co-routines are executed in the idle task using the idle task hook. */
    /* vCoRoutineSchedule(); */ /* Comment this out if not using Co-routines. */

    if(stop_x4_read) {
        printf("Idle task end\n"); 
        fflush(stdout);

        end_application();
    }
    /* Makes the process more agreeable when using the Posix simulator. */
}

void vMainQueueSendPassed( void )
{
    /* This is just an example implementation of the "queue send" trace hook. */
}

#endif

int main() {

    stop_x4_read = 0;

    if(signal(SIGINT, sig_handler) == SIG_ERR){
        printf("can't catch SIGINT\n");
    }

    if(signal(SIGABRT, sig_handler) == SIG_ERR){
        printf("can't catch SIGABRT\n");
    }
    setbuf(stdout, NULL);

    if(radar_task_init()){
        end_application();
        return -1;
    }

#if USE_FREERTOS_TASKS
    if( file_task_init() ) {
#else
    pthread_t file_thread_id;
    if (file_task_init (&file_thread_id) ) {
#endif
        printf("Error initializing file task\n");
        end_application();
        return -1;
    }
#if USE_FREERTOS_TASKS

    vTaskStartScheduler();

#else

    for (;;) {

        pause ();

    }
#endif

    // Should never get here
    printf("Exit from app \n");

    // TODO clean up if needed

    return 0;

}
