/**
 * @file
 *
 *
 */

#include "task_hostcom.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "xtcompiler.h"
#include "protocol_target.h"
#include <string.h>
#include <arm_math.h>
#include "xep_dispatch_messages.h"
#include "xep_hal.h"
#include "xtmemory.h"

#define TASK_HOSTCOM_STACK_SIZE            (2000)
#define TASK_HOSTCOM_PRIORITY        (tskIDLE_PRIORITY + 3)


static void task_hostcom(void *pvParameters);

static uint32_t process_dispatch_message_hostcom_send(XepDispatch_t* dispatch, XepDispatchMessage_t* dispatch_message, XepHostComMCPUserReference_t* mcp_user_reference);
static void mcp_handle_protocol_packet(void * userData, const unsigned char * data, unsigned int length);
static void mcp_handle_protocol_error(void * userData, unsigned int error);
static void mcp_protocol_messagebuild_callback(unsigned char byte, void * user_data);


uint32_t task_hostcom_init(XepDispatch_t* dispatch)
{
	TaskHandle_t h_task_hostcom;

	xTaskCreate(task_hostcom, (const char * const) "HostCom", TASK_HOSTCOM_STACK_SIZE, (void*)dispatch, TASK_HOSTCOM_PRIORITY, &h_task_hostcom);

	return 0;
}

static void task_hostcom(void *pvParameters)
{
	XepDispatch_t* dispatch = (XepDispatch_t*)pvParameters;

	QueueHandle_t dispatch_queue;
	dispatch_register(&dispatch_queue, dispatch, 20);
	dispatch_subscribe(dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, (void*)dispatch_queue);

	unsigned char mcp_instance_memory[getInstanceSize()];

    // OJE: This works OK sending bytes, but not convenient to be forced to put it here.
    uint8_t mcp_protocol_buffer[7000]  __attribute__ ((aligned (4)));

    // OJE: These compile but when sending bytes, not all bytes are sent.
    //uint8_t* mcp_protocol_buffer = xtmemory_malloc_os(500);
    //uint8_t* mcp_protocol_buffer  __attribute__ ((aligned (4)));
    //mcp_protocol_buffer = xtmemory_malloc_os(500);

    XepHostComMCPUserReference_t mcp_user_reference;
	mcp_user_reference.hostcom_task_handle = xTaskGetCurrentTaskHandle();
	mcp_user_reference.dispatch = dispatch;
	mcp_user_reference.messagebuild_buffer = mcp_protocol_buffer;
	mcp_user_reference.messagebuild_index = 0;
	mcp_user_reference.messagebuild_length = sizeof(mcp_protocol_buffer);
	xtProtocol* mcp_protocol_instance = createApplicationProtocol(&mcp_handle_protocol_packet, &mcp_handle_protocol_error, (void*)&mcp_user_reference, mcp_instance_memory, mcp_protocol_buffer, sizeof(mcp_protocol_buffer));
	UNUSED(mcp_protocol_instance);

	//status =
        xtio_serial_com_init(mcp_protocol_instance);

	XepDispatchMessage_t dispatch_message;
	uint32_t notify_value;
	for (;;)
	{
		xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
			0xffffffff, /* Reset the notification value to 0 on exit. */
			&notify_value, /* Notified value pass out. */
			500 / portTICK_PERIOD_MS );  /* Block indefinitely. */

		if (notify_value & dispatch->notify_value)
		{
			while ( xQueueReceive( dispatch_queue, &dispatch_message, 0) )
			{
				if (dispatch_message.tag == XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND)
				{
					process_dispatch_message_hostcom_send(dispatch, &dispatch_message, &mcp_user_reference);
				}
			}
		}
		if (notify_value == 0) // Timeout
		{

		}
	}
}

static uint32_t process_dispatch_message_hostcom_send(XepDispatch_t* dispatch, XepDispatchMessage_t* dispatch_message, XepHostComMCPUserReference_t* mcp_user_reference)
{
    XepDispatchMessageContentCommon_t* message_common = (XepDispatchMessageContentCommon_t*)dispatch_message->memoryblock->buffer;
	if ((dispatch_message->length == sizeof(XepDispatchMessageContentHostcomAck_t)) && (message_common->content_ref == XDMCR_HOSTCOM_ACK))
	{
		mcp_user_reference->messagebuild_index = 0;
		createAckCommand(mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
		xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
	}
	else if ((dispatch_message->length == sizeof(XepDispatchMessageContentHostcomError_t)) && (message_common->content_ref == XDMCR_HOSTCOM_ERROR))
	{
        XepDispatchMessageContentHostcomError_t* message_content = (XepDispatchMessageContentHostcomError_t*)dispatch_message->memoryblock->buffer;
		mcp_user_reference->messagebuild_index = 0;
		createErrorCommand(message_content->errorcode, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
		xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
	}
	else if ((dispatch_message->length == sizeof(XepDispatchMessageContentHostcomPong_t)) && (message_common->content_ref == XDMCR_HOSTCOM_PONG))
	{
		XepDispatchMessageContentHostcomPong_t* message_content = (XepDispatchMessageContentHostcomPong_t*)dispatch_message->memoryblock->buffer;
		mcp_user_reference->messagebuild_index = 0;
		createPongCommand(message_content->pongval, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
		xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
	}
    else if ((dispatch_message->length == sizeof(XepDispatchMessageContentHostcomSystem_t)) && (message_common->content_ref == XDMCR_HOSTCOM_SYSTEM))
    {
        XepDispatchMessageContentHostcomSystem_t* message_content = (XepDispatchMessageContentHostcomSystem_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createSystemCommand(message_content->contentid, NULL, 0, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_DATA_STRING) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomDataString_t* message_content = (XepDispatchMessageContentHostcomDataString_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createDataStringnCommand(message_content->contentid, message_content->info, message_content->data, message_content->length, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_DATA_BYTE) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomDataByte_t* message_content = (XepDispatchMessageContentHostcomDataByte_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createDataByteCommand(message_content->contentid, message_content->info, message_content->data, message_content->length, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_DATA_FLOAT) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomDataFloat_t* message_content = (XepDispatchMessageContentHostcomDataFloat_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createDataFloatCommand(message_content->contentid, message_content->info, message_content->data, message_content->length, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_REPLY_BYTE) && (dispatch_message->length == message_common->message_size))
    {
	    XepDispatchMessageContentHostcomReplyByte_t* message_content = (XepDispatchMessageContentHostcomReplyByte_t*)dispatch_message->memoryblock->buffer;
	    mcp_user_reference->messagebuild_index = 0;
	    createReplyByteCommand(message_content->contentid, message_content->info, message_content->data, message_content->length, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
	    xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_REPLY_INT) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomReplyInt_t* message_content = (XepDispatchMessageContentHostcomReplyInt_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createReplyIntCommand(message_content->contentid, message_content->info, message_content->data, message_content->length, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_REPLY_STRING) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomReplyString_t* message_content = (XepDispatchMessageContentHostcomReplyString_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createReplyStringnCommand(message_content->contentid, message_content->info, message_content->data, message_content->length, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_REPLY_FLOAT) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomReplyFloat_t* message_content = (XepDispatchMessageContentHostcomReplyFloat_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createReplyFloatCommand(message_content->contentid, message_content->info, message_content->data, message_content->length, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_APPDATA_SLEEP) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomAppdataSleep_t* message_content = (XepDispatchMessageContentHostcomAppdataSleep_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createAppdataSleepCommand(message_content->counter, message_content->state_code, message_content->state_data, message_content->distance, message_content->signal_quality, message_content->movement_slow, message_content->movement_fast, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_APPDATA_RESPIRATION) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomAppdataRespiration_t* message_content = (XepDispatchMessageContentHostcomAppdataRespiration_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createAppdataRespirationCommand(message_content->counter, message_content->state_code, message_content->state_data, message_content->distance, message_content->movement, message_content->signal_quality, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_APPDATA_TRUEPRESENCE_SINGLE) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomAppdataTruePresenceSingle_t* message_content = (XepDispatchMessageContentHostcomAppdataTruePresenceSingle_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createAppdataTruePresenceSingleCommand(message_content->counter, message_content->presence_state, message_content->distance, message_content->direction,  message_content->signal_quality, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_APPDATA_TRUEPRESENCE_MOVINGLIST) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomAppdataTruePresenceMovinglist_t* message_content = (XepDispatchMessageContentHostcomAppdataTruePresenceMovinglist_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createAppdataTruePresenceMovinglistCommand(message_content->counter, message_content->presence_state, message_content->movement_interval_count, message_content->detection_count, message_content->movement_slow_item, message_content->movement_fast_item, message_content->detection_distance, message_content->detection_radar_cross_section, message_content->detection_velocity, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_APPDATA_PROFILEPARAMETERFILE) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomAppdataProfileParameterFile_t* message_content = (XepDispatchMessageContentHostcomAppdataProfileParameterFile_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createAppdataProfileParameterFileCommand(message_content->filename_length, message_content->data_length, message_content->filename, message_content->data, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_APPDATA_BASEBAND_AMPLITUDE_PHASE) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomAppdataBasebandAmplitudePhase_t* message_content = (XepDispatchMessageContentHostcomAppdataBasebandAmplitudePhase_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createAppdataBasebandAmplitudePhaseCommand(message_content->counter, message_content->num_of_bins, message_content->bin_length, message_content->sampling_frequency, message_content->carrier_frequency, message_content->range_offset, message_content->data_amplitude, message_content->data_phase, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_HOSTCOM_APPDATA_BASEBAND_IQ) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentHostcomAppdataBasebandIQ_t* message_content = (XepDispatchMessageContentHostcomAppdataBasebandIQ_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;
        createAppdataBasebandIQCommand(message_content->counter, message_content->num_of_bins, message_content->bin_length, message_content->sampling_frequency, message_content->carrier_frequency, message_content->range_offset, message_content->signal_i, message_content->signal_q, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
        xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
    }
    else if ((message_common->content_ref == XDMCR_RADARDATA_FRAME) && (dispatch_message->length == message_common->message_size))
    {
        XepDispatchMessageContentRadardataFramePacket_t* message_content = (XepDispatchMessageContentRadardataFramePacket_t*)dispatch_message->memoryblock->buffer;
        mcp_user_reference->messagebuild_index = 0;

		if (1) // Use for now. Switch to NoEscape when all are ready.
		{
			createDataFloatCommand(ID_RAW_FRAME_BUFFER, message_content->framecounter, message_content->framedata, message_content->bin_count, mcp_protocol_messagebuild_callback, (void*)mcp_user_reference);
			xtio_host_send(mcp_user_reference->messagebuild_buffer, mcp_user_reference->messagebuild_index);
			
		}
		else
		{
			// Create noeascape packet header and prepend to data. Memory already allocated.
			// TODO: Make more generic. Prototype for now.
			uint8_t* mcp_packet_data = ((uint8_t*) dispatch_message->memoryblock->buffer) + sizeof(XepDispatchMessageContentRadardataFramePacket_t);

			uint32_t mcp_packet_header_length = 3+5*4;
			uint8_t mcp_header_buf[mcp_packet_header_length];
			uint32_t offset = 0;

			uint32_t* mcpf_start = (uint32_t*)&mcp_header_buf[offset]; offset += 4;
			uint32_t* mcpf_packet_length = (uint32_t*)&mcp_header_buf[offset]; offset += 4;
			uint8_t* mcpf_packet_crc = (uint8_t*)&mcp_header_buf[offset]; offset += 1;
			uint8_t* mcpf_spr = (uint8_t*)&mcp_header_buf[offset]; offset += 1;
			uint8_t* mcpf_sprd = (uint8_t*)&mcp_header_buf[offset]; offset += 1;
			uint32_t* mcpf_contentid = (uint32_t*)&mcp_header_buf[offset]; offset += 4;
			uint32_t* mcpf_info = (uint32_t*)&mcp_header_buf[offset]; offset += 4;
			uint32_t* mcpf_data_length = (uint32_t*)&mcp_header_buf[offset]; offset += 4;

			*mcpf_start = XTS_FLAGSEQUENCE_START_NOESCAPE;
			*mcpf_packet_length = 3 + 4*4 + dispatch_message->length - sizeof(XepDispatchMessageContentRadardataFramePacket_t);
			*mcpf_packet_crc = 0;
			*mcpf_spr = XTS_SPR_DATA;
			*mcpf_sprd = XTS_SPRD_FLOAT;
			*mcpf_contentid = ID_RAW_FRAME_BUFFER;
			*mcpf_info = message_content->framecounter;
			*mcpf_data_length = message_content->bin_count;

			uint8_t* mcp_packet_header = mcp_packet_data - mcp_packet_header_length;
			memcpy(mcp_packet_header, mcp_header_buf, mcp_packet_header_length);

			xtio_host_send(mcp_packet_header,  4 + *mcpf_packet_length); // StartSequence + Packet
		}
    }
    dispatch_release_message(dispatch, dispatch_message);
	return 0;
}

static void mcp_handle_protocol_packet(void * userData, const unsigned char * data, unsigned int length)
{
	XepHostComMCPUserReference_t* mcp_user_reference = (XepHostComMCPUserReference_t*)userData;

    MemoryBlock_t* memoryblock;
    if (XEP_ERROR_OK != dispatch_get_message_memoryblock(&memoryblock, mcp_user_reference->dispatch, length))
	{
		// TODO: Some error handling.
		return;
	}
    memcpy(memoryblock->buffer, data, length);
    dispatch_send_message(NULL, mcp_user_reference->dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_RX, memoryblock, length);
}

static void mcp_handle_protocol_error(void * userData, unsigned int error)
{
	XepHostComMCPUserReference_t* mcp_user_reference = (XepHostComMCPUserReference_t*)userData;
	UNUSED(mcp_user_reference);

}

static void mcp_protocol_messagebuild_callback(unsigned char byte, void * user_data)
{
	XepHostComMCPUserReference_t* mcp_user_reference = (XepHostComMCPUserReference_t*)user_data;
	if (mcp_user_reference->messagebuild_index >= mcp_user_reference->messagebuild_length)
	{
		// ERROR. Buffer overflow.
		return;
	}
	mcp_user_reference->messagebuild_buffer[mcp_user_reference->messagebuild_index++] = byte;
}
