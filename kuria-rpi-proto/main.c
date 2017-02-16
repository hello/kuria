#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

bool stop_x4_read = 0;

void sig_handler(int sig) {
    if(sig == SIGINT)  {
        stop_x4_read = 1;
    }
}

int main() {
    if(signal(SIGINT, sig_handler) == SIG_ERR){
        printf("can't catch SIGINT\n");
    }
    printf("X4 Test start...\n");

    // Open file to save data
    // Initialize x4 module
    //

    while(!stop_x4_read) {
        // poll x4 for data
        //
        sleep(1);
    }
    printf("Ending X4 Test...\n");
    return 0;

}
