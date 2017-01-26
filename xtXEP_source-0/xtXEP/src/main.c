/**
 * @file
 *
 * @brief main() function for Radar Application
 *
 */

#include "FreeRTOS.h"
#include "task.h"
#include "xep_hal.h"
#include "xep_application.h"
#include "task_monitor.h"
#include "task_hostcom.h"
#include "task_radar.h"
#include "xtcompiler.h"


// C++ requires these. Dummy for now.
int _write(int file, const char *ptr, int len);
int _write(int file, const char *ptr, int len){return 0;}
int _read(int file, const char *ptr, int len);
int _read(int file, const char *ptr, int len){return 0;}

// Adressing linker problem in product projects:
// http://stackoverflow.com/questions/25851138/how-to-include-syscalls-c-from-a-separate-library-file
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic push
#define FORCE_LINK(x) void* __ ## x ## _force_link =(void*)&x
#pragma GCC diagnostic pop
extern void _getpid(int status);


int main(void)
{
	FORCE_LINK(_getpid); // Force early link of syscalls.

	int status = 0;

    status = xt_board_init_ext(0);

    status = xtio_led_set_state(XTIO_LED_RED, XTIO_LED_ON);
    status = xtio_led_set_state(XTIO_LED_RED, XTIO_LED_OFF);
    status = xtio_led_set_state(XTIO_LED_GREEN, XTIO_LED_ON);

    // Create necessary tasks
	void* dispatch = NULL;
	void* x4driver = NULL;
    status = xep_init((void*)&dispatch);
    status = task_radar_init((void*)&x4driver, dispatch);
	//status = task_monitor_init(dispatch);
	status = task_hostcom_init(dispatch);
    status = task_application_init(dispatch, x4driver);

    // Start the RTOS scheduler.
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for (;;) {
	}

	return status;
}
