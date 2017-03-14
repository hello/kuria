/**
 * @file
 *
 * 
 */

#include "xep_dispatch_messages.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

uint32_t dispatch_message_hostcom_send_ack(XepDispatch_t* dispatch)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomAck_t);
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomAck_t* message_content = (XepDispatchMessageContentHostcomAck_t*)memoryblock->buffer;
	message_content->common.content_ref = XDMCR_HOSTCOM_ACK;
	message_content->common.message_size = size;
	uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_error(XepDispatch_t* dispatch, uint32_t errorcode)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomError_t);
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomError_t* message_content = (XepDispatchMessageContentHostcomError_t*)memoryblock->buffer;
	message_content->common.content_ref = XDMCR_HOSTCOM_ERROR;
	message_content->common.message_size = size;
	message_content->errorcode = errorcode;
	uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_pong(XepDispatch_t* dispatch, uint32_t pongval)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomPong_t);
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomPong_t* message_content = (XepDispatchMessageContentHostcomPong_t*)memoryblock->buffer;
	message_content->common.content_ref = XDMCR_HOSTCOM_PONG;
	message_content->common.message_size = size;
	message_content->pongval = pongval;
	uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_appdata_sleep(XepDispatch_t* dispatch, uint32_t counter, uint32_t state_code, float state_data, float distance, uint32_t signal_quality, float movement_slow, float movement_fast)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomAppdataSleep_t);
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomAppdataSleep_t* message_content = (XepDispatchMessageContentHostcomAppdataSleep_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_APPDATA_SLEEP;
    message_content->common.message_size = size;
    message_content->counter = counter;
    message_content->state_code = state_code;
    message_content->state_data = state_data;
    message_content->distance = distance;
    message_content->signal_quality = signal_quality;
    message_content->movement_slow = movement_slow;
    message_content->movement_fast = movement_fast;
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_appdata_respiration(XepDispatch_t* dispatch, uint32_t counter, uint32_t state_code, uint32_t state_data, float distance, float movement, uint32_t signal_quality)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomAppdataRespiration_t);
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomAppdataRespiration_t* message_content = (XepDispatchMessageContentHostcomAppdataRespiration_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_APPDATA_RESPIRATION;
    message_content->common.message_size = size;
    message_content->counter = counter;
    message_content->state_code = state_code;
    message_content->state_data = state_data;
    message_content->distance = distance;
    message_content->movement = movement;
    message_content->signal_quality = signal_quality;
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_appdata_truepresence_single(XepDispatch_t* dispatch, uint32_t counter, uint32_t presence_state, float distance, uint8_t direction, uint32_t signal_quality)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomAppdataTruePresenceSingle_t);
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomAppdataTruePresenceSingle_t* message_content = (XepDispatchMessageContentHostcomAppdataTruePresenceSingle_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_APPDATA_TRUEPRESENCE_SINGLE;
    message_content->common.message_size = size;
    message_content->counter = counter;
    message_content->presence_state = presence_state;
    message_content->distance = distance;
    message_content->direction = direction;
    message_content->signal_quality = signal_quality;
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_appdata_truepresence_movinglist(XepDispatch_t* dispatch, uint32_t counter, uint32_t presence_state, uint32_t movement_interval_count, uint32_t detection_count, const float* movement_slow_item, const float* movement_fast_item, const float* detection_distance, const float* detection_radar_cross_section, const float* detection_velocity)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomAppdataTruePresenceMovinglist_t) + movement_interval_count * sizeof(float) * 2 + detection_count * sizeof(float) * 3;
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomAppdataTruePresenceMovinglist_t* message_content = (XepDispatchMessageContentHostcomAppdataTruePresenceMovinglist_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_APPDATA_TRUEPRESENCE_MOVINGLIST;
    message_content->common.message_size = size;
    message_content->counter = counter;
    message_content->presence_state = presence_state;
    message_content->movement_interval_count = movement_interval_count;
    message_content->detection_count = detection_count;
    message_content->movement_slow_item = (float*) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomAppdataTruePresenceMovinglist_t) );
    message_content->movement_fast_item = (float*) ( (uint32_t)(void*)message_content->movement_slow_item + movement_interval_count * sizeof(float) );
    message_content->detection_distance = (float*) ( (uint32_t)(void*)message_content->movement_fast_item + movement_interval_count * sizeof(float) );
    message_content->detection_radar_cross_section = (float*) ( (uint32_t)(void*)message_content->detection_distance + detection_count * sizeof(float) );
    message_content->detection_velocity = (float*) ( (uint32_t)(void*)message_content->detection_radar_cross_section + detection_count * sizeof(float) );
    memcpy(message_content->movement_slow_item, movement_slow_item, movement_interval_count * sizeof(float));
    memcpy(message_content->movement_fast_item, movement_fast_item, movement_interval_count * sizeof(float));
    memcpy(message_content->detection_distance, detection_distance, detection_count * sizeof(float));
    memcpy(message_content->detection_radar_cross_section, detection_radar_cross_section, detection_count * sizeof(float));
    memcpy(message_content->detection_velocity, detection_velocity, detection_count * sizeof(float));
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_appdata_baseband_amplitude_phase(XepDispatch_t* dispatch, uint32_t counter, uint32_t num_of_bins, float bin_length, float sampling_frequency, float carrier_frequency, float range_offset, const float* data_amplitude, const float* data_phase)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomAppdataBasebandAmplitudePhase_t) + num_of_bins * sizeof(float) * 2;
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomAppdataBasebandAmplitudePhase_t* message_content = (XepDispatchMessageContentHostcomAppdataBasebandAmplitudePhase_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_APPDATA_BASEBAND_AMPLITUDE_PHASE;
    message_content->common.message_size = size;
    message_content->counter = counter;
    message_content->num_of_bins = num_of_bins;
    message_content->bin_length = bin_length;
    message_content->sampling_frequency = sampling_frequency;
    message_content->carrier_frequency = carrier_frequency;
    message_content->range_offset = range_offset;
    message_content->data_amplitude = (float*) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomAppdataBasebandAmplitudePhase_t) );
    message_content->data_phase = (float*) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomAppdataBasebandAmplitudePhase_t) + num_of_bins * sizeof(float) );
    memcpy(message_content->data_amplitude, data_amplitude, num_of_bins * sizeof(float));
    memcpy(message_content->data_phase, data_phase, num_of_bins * sizeof(float));
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_appdata_baseband_iq(XepDispatch_t* dispatch, uint32_t counter, uint32_t num_of_bins, float bin_length, float sampling_frequency, float carrier_frequency, float range_offset, const float* signal_i, const float* signal_q)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomAppdataBasebandIQ_t) + num_of_bins * sizeof(float) * 2;
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomAppdataBasebandIQ_t* message_content = (XepDispatchMessageContentHostcomAppdataBasebandIQ_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_APPDATA_BASEBAND_IQ;
    message_content->common.message_size = size;
    message_content->counter = counter;
    message_content->num_of_bins = num_of_bins;
    message_content->bin_length = bin_length;
    message_content->sampling_frequency = sampling_frequency;
    message_content->carrier_frequency = carrier_frequency;
    message_content->range_offset = range_offset;
    message_content->signal_i = (float*) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomAppdataBasebandIQ_t) );
    message_content->signal_q = (float*) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomAppdataBasebandIQ_t) + num_of_bins * sizeof(float) );
    memcpy(message_content->signal_i, signal_i, num_of_bins * sizeof(float));
    memcpy(message_content->signal_q, signal_q, num_of_bins * sizeof(float));
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_appdata_profileparameterfile(XepDispatch_t* dispatch, uint32_t filename_length, uint32_t data_length, const char * filename, const char * data)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomAppdataProfileParameterFile_t) + filename_length + data_length;
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomAppdataProfileParameterFile_t* message_content = (XepDispatchMessageContentHostcomAppdataProfileParameterFile_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_APPDATA_PROFILEPARAMETERFILE;
    message_content->common.message_size = size;
    message_content->filename_length = filename_length;
    message_content->data_length = data_length;
    message_content->filename = (unsigned char *) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomAppdataProfileParameterFile_t) );
    message_content->data = (unsigned char *) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomAppdataProfileParameterFile_t) + filename_length );
    memcpy(message_content->filename, filename, filename_length);
    memcpy(message_content->data, data, data_length);
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_system(XepDispatch_t* dispatch, uint32_t contentid)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomSystem_t);
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomSystem_t* message_content = (XepDispatchMessageContentHostcomSystem_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_SYSTEM;
    message_content->common.message_size = size;
    message_content->contentid = contentid;
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_data_string(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, char* format, ...)
{
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, 256, format, args);
    va_end(args);
    int length = strlen(buf);
    return dispatch_message_hostcom_send_data_stringn(dispatch, contentid, info, (unsigned char *)buf, length);
}

uint32_t dispatch_message_hostcom_send_data_stringn(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, unsigned char* data, uint32_t length)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomDataString_t) + length;
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomDataString_t* message_content = (XepDispatchMessageContentHostcomDataString_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_DATA_STRING;
    message_content->common.message_size = size;
    message_content->contentid = contentid;
    message_content->info = info;
    message_content->length = length;
    message_content->data = (unsigned char*) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomDataString_t) );
    memcpy(message_content->data, (void*)data, length);
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_data_byte(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, uint8_t* data, uint32_t length)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomDataByte_t) + length;
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomDataByte_t* message_content = (XepDispatchMessageContentHostcomDataByte_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_DATA_BYTE;
    message_content->common.message_size = size;
    message_content->contentid = contentid;
    message_content->info = info;
    message_content->length = length;
    message_content->data = (uint8_t*) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomDataByte_t) );
    memcpy(message_content->data, (void*)data, length);
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_data_float(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, float* data, uint32_t length)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomDataFloat_t) + sizeof(float)*length;
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomDataFloat_t* message_content = (XepDispatchMessageContentHostcomDataFloat_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_DATA_FLOAT;
    message_content->common.message_size = size;
    message_content->contentid = contentid;
    message_content->info = info;
    message_content->length = length;
    message_content->data = (float*) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomDataFloat_t) );
    memcpy(message_content->data, (void*)data, length*sizeof(float));
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_reply_int(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, int32_t* data, uint32_t length)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomReplyInt_t) + sizeof(int)*length;
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomReplyInt_t* message_content = (XepDispatchMessageContentHostcomReplyInt_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_REPLY_INT;
    message_content->common.message_size = size;
    message_content->contentid = contentid;
    message_content->info = info;
    message_content->length = length;
    message_content->data = (int32_t*) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomReplyInt_t) );
    memcpy(message_content->data, (void*)data, length*sizeof(float));
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_reply_byte(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, uint8_t* data, uint32_t length)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomReplyByte_t) + sizeof(int)*length;
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomReplyByte_t* message_content = (XepDispatchMessageContentHostcomReplyByte_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_REPLY_BYTE;
    message_content->common.message_size = size;
    message_content->contentid = contentid;
    message_content->info = info;
    message_content->length = length;
    message_content->data = (uint8_t*) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomReplyByte_t) );
    memcpy(message_content->data, (void*)data, length*sizeof(float));
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_reply_string(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, char* format, ...)
{
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, 256, format, args);
    va_end(args);
    int length = strlen(buf);
    return dispatch_message_hostcom_send_reply_stringn(dispatch, contentid, info, (unsigned char * )buf, length);
}

uint32_t dispatch_message_hostcom_send_reply_stringn(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, unsigned char* data, uint32_t length)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomReplyString_t) + length;
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomReplyString_t* message_content = (XepDispatchMessageContentHostcomReplyString_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_REPLY_STRING;
    message_content->common.message_size = size;
    message_content->contentid = contentid;
    message_content->info = info;
    message_content->length = length;
    message_content->data = (unsigned char*) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomReplyString_t) );
    memcpy(message_content->data, (void*)data, length);
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}

uint32_t dispatch_message_hostcom_send_reply_float(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, float* data, uint32_t length)
{
    uint32_t size = sizeof(XepDispatchMessageContentHostcomReplyFloat_t) + sizeof(float)*length;
    MemoryBlock_t* memoryblock;
    uint32_t status = memorypoolset_take_auto(&memoryblock, dispatch->memorypoolset, size);
    if (status != XEP_ERROR_OK)
        return status;
    XepDispatchMessageContentHostcomReplyFloat_t* message_content = (XepDispatchMessageContentHostcomReplyFloat_t*)memoryblock->buffer;
    message_content->common.content_ref = XDMCR_HOSTCOM_REPLY_FLOAT;
    message_content->common.message_size = size;
    message_content->contentid = contentid;
    message_content->info = info;
    message_content->length = length;
    message_content->data = (float*) ( (uint32_t)(void*)message_content + sizeof(XepDispatchMessageContentHostcomReplyFloat_t) );
    memcpy(message_content->data, (void*)data, length*sizeof(float));
    uint32_t message_id=0;
    status = dispatch_send_message(&message_id, dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, memoryblock, message_content->common.message_size);
    return status;
}



uint32_t dispatch_message_hostcom_forward_radardata_frame_packet(XepDispatch_t* dispatch, XepDispatchMessage_t* message)
{
    // Repurpuse radar dispatch message directly, with no copy.
    int status = dispatch_forward_message(dispatch, XEP_DISPATCH_MESSAGETAG_HOSTCOM_SEND, message);
    return status;
}

uint32_t dispatch_message_radardata_prepare_frame(XepDispatchMessageContentRadardataFramePacket_t** frame_packet, MemoryBlock_t** memoryblock, XepDispatch_t* dispatch, uint32_t bin_count)
{
    uint32_t size = (uint32_t)sizeof(XepDispatchMessageContentRadardataFramePacket_t) + bin_count*(uint32_t)sizeof(float);
    uint32_t status = dispatch_get_message_memoryblock(memoryblock, dispatch, size);
    if (status != XEP_ERROR_OK)
    {
        return status;
    }
    *frame_packet = (XepDispatchMessageContentRadardataFramePacket_t*)(void*)(*memoryblock)->buffer;
    (*frame_packet)->bin_count = bin_count;
    (*frame_packet)->common.content_ref = XDMCR_RADARDATA_FRAME;
    (*frame_packet)->common.message_size = size;
    (*frame_packet)->framedata = (float*)((uint32_t)(void*)*frame_packet + (uint32_t)sizeof(XepDispatchMessageContentRadardataFramePacket_t));
    return status;
}

