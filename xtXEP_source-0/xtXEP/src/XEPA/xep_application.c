/**
 * @file
 *
 *
 */

#include "xep_application.h"
#include "FreeRTOS.h"
#include <task.h>
#include <queue.h>
#include "protocol.h"
#include "xep_dispatch_messages.h"
#include "module_ui.h"
#include "xep.h"
#include "xtmemory.h"
#include "protocol_parser.h"
#include "xep_application_mcp_callbacks.h"
#include "string.h"
#include "xttoolbox.h"
#include "string.h"
#include "version.h"
#include "task_monitor.h"

#define TASK_APP_STACK_SIZE            (3000)
#define TASK_APP_PRIORITY        (tskIDLE_PRIORITY + 5)


int xep_init_memorypoolset(XepHandle_t* xep) __attribute__ ((weak, alias("xep_init_memorypoolset_default")));
int xep_app_init(XepHandle_t* xep) __attribute__ ((weak, alias("xep_app_init_default")));
int xep_app_iterate(XepHandle_t* xep, uint32_t notify) __attribute__ ((weak, alias("xep_app_iterate_default")));
int xep_process_dispatch_message(XepHandle_t* xep, XepDispatchMessage_t* dispatch_message) __attribute__ ((weak, alias("xep_process_dispatch_message_default")));

int xep_init_memorypoolset_default(XepHandle_t* xep);
int xep_app_init_default(XepHandle_t* xep);
int xep_app_iterate_default(XepHandle_t* xep, uint32_t notify);

static void task_application(void *pvParameters);

typedef struct
{
    TaskHandle_t application_task_handle;
} XepInternalInfo_t;



int xep_debug_control(XepHandle_t* xep, uint32_t debug_control)
{
    XepInternalInfo_t* xep_internal_info = xep->internal;
    if (debug_control == XDC_APPLICATION_TASK_HIGH_PRIORITY)
    {
        vTaskPrioritySet(xep_internal_info->application_task_handle, tskIDLE_PRIORITY + 7);
    }
    else
    {
        vTaskPrioritySet(xep_internal_info->application_task_handle, TASK_APP_PRIORITY);
    }
    return 0;
}

int xep_get_systeminfo_versionlist(char* destination, uint32_t max_length)
{
    uint32_t add_length = strlen(PRODUCT_NAME) + 1 + strlen(VERSION_STRING) + 1 + strlen(X4C51FW_PRODUCT_NAME) + 1 + strlen(X4C51FW_VERSION_STRING);
    if (max_length < add_length) return 0;
    sprintf(destination, "%s:%s;%s:%s", PRODUCT_NAME, VERSION_STRING, X4C51FW_PRODUCT_NAME, X4C51FW_VERSION_STRING);
    return add_length;
}

uint32_t xep_init(XepDispatch_t** dispatch)
{
	int status = 0;
	uint32_t size = dispatch_get_instance_size();
	void* instance_memory = xtmemory_malloc_default(size);
	status = dispatch_create(dispatch, instance_memory);
	status = dispatch_set_notify_value(*dispatch, 0x10000000);

    // Set up memory poolset
    MemoryPoolSet_t* memorypoolset;
    size = memorypoolset_get_instance_size();
    instance_memory = xtmemory_malloc_default(size);
    status = memorypoolset_create(&memorypoolset, instance_memory);

	(*dispatch)->memorypoolset = memorypoolset;

	return status;
}

uint32_t task_application_init(XepDispatch_t* dispatch, X4Driver_t* x4driver)
{
    XepHandle_t* xep = xtmemory_malloc_default(sizeof(XepHandle_t));
    xep->dispatch = dispatch;
    xep->x4driver = x4driver;
    xep->user_reference = NULL;
    xep->internal = xtmemory_malloc_default(sizeof(XepInternalInfo_t));
    void* instance_memory = xtmemory_malloc_default(getMcpParserInstanceSize());
    xep->mcp_parser = createMcpParser((void*)xep, instance_memory);
    xep->mcp_parser->user_data = (void*)xep;

    xep->mcp_parser->onMcpPing = onMcpPing;
    xep->mcp_parser->onMcpUnknown = onMcpUnknown;
    xep->mcp_parser->onMcpSystemRunTest = onMcpSystemRunTest;
    xep->mcp_parser->onMcpModuleReset = onMcpModuleReset;
    xep->mcp_parser->onMcpStartBootloader = onMcpStartBootloader;
    xep->mcp_parser->onMcpGetSystemInfo = onMcpGetSystemInfo;

    xep->mcp_parser->onMcpSetIOPinControl = onMcpSetIOPinControl;
    xep->mcp_parser->onMcpSetIOPinValue = onMcpSetIOPinValue;
    xep->mcp_parser->onMcpGetIOPinValue = onMcpGetIOPinValue;

    xep->mcp_parser->onMcpX4DriverSetFps = onMcpX4DriverSetFps;
    xep->mcp_parser->onMcpX4DriverSetIterations = onMcpX4DriverSetIterations;
    xep->mcp_parser->onMcpX4DriverSetPulsesPerStep = onMcpX4DriverSetPulsesPerStep;
    xep->mcp_parser->onMcpX4DriverSetDownconversion = onMcpX4DriverSetDownconversion;
    xep->mcp_parser->onMcpX4DriverSetFrameArea = onMcpX4DriverSetFrameArea;
    xep->mcp_parser->onMcpX4DriverInit = onMcpX4DriverInit;
    xep->mcp_parser->onMcpX4DriverSetDacStep = onMcpX4DriverSetDacStep;
    xep->mcp_parser->onMcpX4DriverSetDacMin = onMcpX4DriverSetDacMin;
    xep->mcp_parser->onMcpX4DriverSetDacMax = onMcpX4DriverSetDacMax;
    xep->mcp_parser->onMcpX4DriverSetFrameAreaOffset = onMcpX4DriverSetFrameAreaOffset;
    xep->mcp_parser->onMcpX4DriverSetEnable = onMcpX4DriverSetEnable;
    xep->mcp_parser->onMcpX4DriverSetTxCenterFrequency = onMcpX4DriverSetTxCenterFrequency;
    xep->mcp_parser->onMcpX4DriverSetTxPower = onMcpX4DriverSetTxPower;
    xep->mcp_parser->onMcpX4DriverGetFps = onMcpX4DriverGetFps;
    xep->mcp_parser->onMcpX4DriverSetSpiRegister = onMcpX4DriverSetSpiRegister;
    xep->mcp_parser->onMcpX4DriverGetSpiRegister = onMcpX4DriverGetSpiRegister;
    xep->mcp_parser->onMcpX4DriverSetPifRegister = onMcpX4DriverSetPifRegister;
    xep->mcp_parser->onMcpX4DriverGetPifRegister = onMcpX4DriverGetPifRegister;
    xep->mcp_parser->onMcpX4DriverSetXifRegister = onMcpX4DriverSetXifRegister;
    xep->mcp_parser->onMcpX4DriverGetXifRegister = onMcpX4DriverGetXifRegister;
    xep->mcp_parser->onMcpX4DriverSetPrfDiv = onMcpX4DriverSetPrfDiv;
    xep->mcp_parser->onMcpX4DriverGetPrfDiv = onMcpX4DriverGetPrfDiv;
    xep->mcp_parser->onMcpX4DriverGetFrameArea = onMcpX4DriverGetFrameArea;
    xep->mcp_parser->onMcpX4DriverGetFrameAreaOffset = onMcpX4DriverGetFrameAreaOffset;

    int status = xep_init_memorypoolset((void*)xep);
	UNUSED(status);

	TaskHandle_t task_handle;
    xTaskCreate(task_application, (const char * const) "App", TASK_APP_STACK_SIZE, (void*)xep, TASK_APP_PRIORITY, &task_handle);
    XepInternalInfo_t* xep_internal_info = (XepInternalInfo_t*)xep->internal;
    xep_internal_info->application_task_handle = task_handle;

	return 0;
}

static void task_application(void *pvParameters)
{
	XepHandle_t* xep = (XepHandle_t*)pvParameters;

	uint32_t status;
	UNUSED(status);

	QueueHandle_t dispatch_queue;
	dispatch_register(&dispatch_queue, xep->dispatch, 20);
	dispatch_subscribe(xep->dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_RX, (void*)dispatch_queue);
	dispatch_subscribe(xep->dispatch, XEP_DISPATCH_MESSAGETAG_RADAR_DATA, (void*)dispatch_queue);

    // Add delay to make sure comms are up.
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    monitor_task_t * monitor_task_handle;
    status = monitor_task_register(&monitor_task_handle, 1000);

    // Send system booting message
    dispatch_message_hostcom_send_system(xep->dispatch, XTS_SPRS_BOOTING);

    xep_app_init(xep);

    // Send system ready message
    dispatch_message_hostcom_send_system(xep->dispatch, XTS_SPRS_READY);

	XepDispatchMessage_t dispatch_message;
	uint32_t notify_value;
	for (;;)
	{
        status = monitor_task_alive(monitor_task_handle);

		xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
			0xffffffff, /* Reset the notification value to 0 on exit. */
			&notify_value, /* Notified value pass out. */
            500 / portTICK_PERIOD_MS );  /* Block indefinitely. */

		if (notify_value & xep->dispatch->notify_value)
		{
			// Message from dispatch
			while ( xQueueReceive( dispatch_queue, &dispatch_message, 0 ) )
			{
				xep_process_dispatch_message(xep, &dispatch_message);
			}
		}
		if (notify_value == 0) // Timeout
		{

		}
		xep_app_iterate(xep, notify_value);
	}
}


int xep_init_memorypoolset_default(XepHandle_t* xep)
{
	// Default memory pool set. Facilitate basic dispatch messages, and radar data to host.
	int status = 0;
	uint32_t block_size;
	uint32_t block_count;
	uint32_t size;
	void* instance_memory;

    if (xt_get_operation_mode() == XT_OPMODE_FACTORY)
    {
        block_size = 1000;
        block_count = 2;
        size = memorypool_get_instance_size(block_size, block_count);
        instance_memory = xtmemory_malloc_default(size);
        status = memorypoolset_add_memorypool(NULL, xep->dispatch->memorypoolset, instance_memory, block_size, block_count);

        block_size = 100;
        block_count = 10;
        size = memorypool_get_instance_size(block_size, block_count);
        instance_memory = xtmemory_malloc_default(size);
        status = memorypoolset_add_memorypool(NULL, xep->dispatch->memorypoolset, instance_memory, block_size, block_count);
    } else
    {
        block_size = 7000;
        block_count = 2;
        size = memorypool_get_instance_size(block_size, block_count);
        instance_memory = xtmemory_malloc_slow(size);
        status = memorypoolset_add_memorypool(NULL, xep->dispatch->memorypoolset, instance_memory, block_size, block_count);

        block_size = 100;
        block_count = 10;
        size = memorypool_get_instance_size(block_size, block_count);
        instance_memory = xtmemory_malloc_slow(size);
        status = memorypoolset_add_memorypool(NULL, xep->dispatch->memorypoolset, instance_memory, block_size, block_count);
    }

	return status;
}

int xep_app_init_default(XepHandle_t* xep)
{
	return 0;
}


int xep_app_iterate_default(XepHandle_t* xep, uint32_t notify)
{
	return 0;
}

int xep_process_dispatch_message_default(XepHandle_t* xep, XepDispatchMessage_t* dispatch_message)
{
	if (dispatch_message->tag == XEP_DISPATCH_MESSAGETAG_HOSTCOM_RX)
	{
        mcpParseMessage(xep->mcp_parser, (uint8_t*)dispatch_message->memoryblock->buffer, dispatch_message->length);
        dispatch_release_message(xep->dispatch, dispatch_message);
	}
	else if (dispatch_message->tag == XEP_DISPATCH_MESSAGETAG_RADAR_DATA)
	{
        XepDispatchMessageContentCommon_t* message_common = (XepDispatchMessageContentCommon_t*)dispatch_message->memoryblock->buffer;
		if ((message_common->content_ref == XDMCR_RADARDATA_FRAME) && (dispatch_message->length == message_common->message_size))
		{
			XepDispatchMessageContentRadardataFramePacket_t* frame_packet = (XepDispatchMessageContentRadardataFramePacket_t*)dispatch_message->memoryblock->buffer;
			UNUSED(frame_packet);

            dispatch_message_hostcom_forward_radardata_frame_packet(xep->dispatch, dispatch_message);
            module_ui_led_set_color(0, frame_packet->framecounter%2, 0);
        }
        else
        {
            dispatch_release_message(xep->dispatch, dispatch_message);
        }
	}
    else
    {
        dispatch_release_message(xep->dispatch, dispatch_message);
    }


	return 0;
}
