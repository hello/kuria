#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "kuria_config.h"


#include <string.h>
#include "radar_task.h"
#include "hlo_notify.h"



bool stop_x4_read = 0;
extern hlo_notify_t radar_task_notify;
void end_application(void); 
void sig_handler(int sig) {
    if(sig == SIGINT || sig == SIGABRT)  {
        stop_x4_read = 1;
        end_application();
        exit(0);
    }
}

void end_application(void) {

    // End radar task
    hlo_notify_send (&radar_task_notify, 0x10);

}


int main() {

    stop_x4_read = 0;

    if(signal(SIGINT, sig_handler) == SIG_ERR){
        printf("can't catch SIGINT\n");
    }
/*
    if(signal(SIGABRT, sig_handler) == SIG_ERR){
        printf("can't catch SIGABRT\n");
    }
    */
    setbuf(stdout, NULL);

    if (radar_task_init () ) {
        end_application();
        return -1;
    }

    // enable interrupts
    radar_task_en_intr ();

    // run radar data publisher
    radar_task ();

    // Should never get here
    printf("Exit from app \n");

    return 0;

}
