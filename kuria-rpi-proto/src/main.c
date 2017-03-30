#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "kuria_config.h"


#include <string.h>
#include "file_save.h"
#include "radar_task.h"

bool stop_x4_read = 0;

void end_application(void); 
void sig_handler(int sig) {
    if(sig == SIGINT || sig == SIGABRT)  {
        stop_x4_read = 1;
        end_application();
        exit(0);
    }
}

void end_application(void) {

    // Close files
    file_close();

    // End radar task
    radar_task_end ();

}

void vApplicationTickHook (void) {
}

void vApplicationIdleHook (void) {
}

void vMainQueueSendPassed( void )
{
    /* This is just an example implementation of the "queue send" trace hook. */
}


int main() {

    stop_x4_read = 0;

    if(signal(SIGINT, sig_handler) == SIG_ERR){
        printf("can't catch SIGINT\n");
    }

    if(signal(SIGABRT, sig_handler) == SIG_ERR){
        printf("can't catch SIGABRT\n");
    }
    setbuf(stdout, NULL);

    pthread_t radar_task_thread_id;
    if (radar_task_init (&radar_task_thread_id) ) {
        end_application();
        return -1;
    }

    pthread_t file_thread_id;
    if (file_task_init (&file_thread_id) ) {
        printf("Error initializing file task\n");
        end_application();
        return -1;
    }
    radar_task_en_intr ();

    for (;;) {

        pause ();

    }

    // Should never get here
    printf("Exit from app \n");

    // TODO clean up if needed

    return 0;

}
