/**
 * @file
 *
 *
 */

#ifndef XEP_DISPATCH_MESSAGES_H
#define XEP_DISPATCH_MESSAGES_H

#include "xep_dispatch.h"
#include <stdint.h>
/*
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#include <arm_math.h>
#pragma GCC diagnostic pop
*/


//==============================================================================
// Codes used by dispatch'er to identify dispatch message content.

typedef enum
{
    XDMCR_SIMPLERESULT = 1,
    XDMCR_HOSTCOM_ACK,
    XDMCR_HOSTCOM_ERROR,
    XDMCR_HOSTCOM_PONG,
    XDMCR_HOSTCOM_APPDATA_SLEEP,
    XDMCR_HOSTCOM_APPDATA_RESPIRATION,
    XDMCR_HOSTCOM_APPDATA_TRUEPRESENCE_SINGLE,
    XDMCR_HOSTCOM_APPDATA_TRUEPRESENCE_MOVINGLIST,
    XDMCR_HOSTCOM_APPDATA_BASEBAND_AMPLITUDE_PHASE,
    XDMCR_HOSTCOM_APPDATA_BASEBAND_IQ,
    XDMCR_HOSTCOM_APPDATA_PROFILEPARAMETERFILE,
    XDMCR_HOSTCOM_SYSTEM,
    XDMCR_HOSTCOM_DATA_STRING,
    XDMCR_HOSTCOM_DATA_BYTE,
    XDMCR_HOSTCOM_DATA_FLOAT,
    XDMCR_HOSTCOM_REPLY_INT,
    XDMCR_HOSTCOM_REPLY_BYTE,
    XDMCR_HOSTCOM_REPLY_STRING,
    XDMCR_HOSTCOM_REPLY_FLOAT,
    XDMCR_RADARDATA_FRAME
} XepDispatchMessageContentReference_t;


//==============================================================================
// Structs used as containers for message data packages.

typedef struct
{
	uint32_t content_ref;
	uint32_t message_size;
} XepDispatchMessageContentCommon_t;

typedef struct
{
	XepDispatchMessageContentCommon_t common;
	uint32_t status;
} XepDispatchMessageContentSimpleResult_t;


typedef struct
{
    XepDispatchMessageContentCommon_t common;
} XepDispatchMessageContentHostcomAck_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t errorcode;
} XepDispatchMessageContentHostcomError_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t pongval;
} XepDispatchMessageContentHostcomPong_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t contentid;
} XepDispatchMessageContentHostcomSystem_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t contentid;
    uint32_t info;
    uint32_t length;
    uint8_t* data;
} XepDispatchMessageContentHostcomDataString_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t contentid;
    uint32_t info;
    uint32_t length;
    uint8_t* data;
} XepDispatchMessageContentHostcomDataByte_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t contentid;
    uint32_t info;
    uint32_t length;
    float* data;
} XepDispatchMessageContentHostcomDataFloat_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t contentid;
    uint32_t info;
    uint32_t length;
    int32_t* data;
} XepDispatchMessageContentHostcomReplyInt_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t contentid;
    uint32_t info;
    uint32_t length;
    uint8_t* data;
} XepDispatchMessageContentHostcomReplyString_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t contentid;
    uint32_t info;
    uint32_t length;
    uint8_t* data;
} XepDispatchMessageContentHostcomReplyByte_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t contentid;
    uint32_t info;
    uint32_t length;
    float* data;
} XepDispatchMessageContentHostcomReplyFloat_t;


typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t counter;
    uint32_t state_code;
    float state_data;
    float distance;
    uint32_t signal_quality;
    float movement_slow;
    float movement_fast;
} XepDispatchMessageContentHostcomAppdataSleep_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t counter;
    uint32_t state_code;
    uint32_t state_data;
    float distance;
    float movement;
    uint32_t signal_quality;
} XepDispatchMessageContentHostcomAppdataRespiration_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t counter;
    uint32_t presence_state;
    float distance;
    uint8_t direction;
    uint32_t signal_quality;
} XepDispatchMessageContentHostcomAppdataTruePresenceSingle_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t counter;
    uint32_t presence_state;
    uint32_t movement_interval_count;
    uint32_t detection_count;
    float* movement_slow_item;
    float* movement_fast_item;
    float* detection_distance;
    float* detection_radar_cross_section;
    float* detection_velocity;
} XepDispatchMessageContentHostcomAppdataTruePresenceMovinglist_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t filename_length;
    uint32_t data_length;
    unsigned char * filename;
    unsigned char * data;
} XepDispatchMessageContentHostcomAppdataProfileParameterFile_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t counter;
    uint32_t num_of_bins;
    float bin_length;
    float sampling_frequency;
    float carrier_frequency;
    float range_offset;
    float* data_amplitude;
    float* data_phase;
} XepDispatchMessageContentHostcomAppdataBasebandAmplitudePhase_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t counter;
    uint32_t num_of_bins;
    float bin_length;
    float sampling_frequency;
    float carrier_frequency;
    float range_offset;
    float* signal_i;
    float* signal_q;
} XepDispatchMessageContentHostcomAppdataBasebandIQ_t;

typedef struct
{
    XepDispatchMessageContentCommon_t common;
    uint32_t framecounter;
    uint32_t bin_count;
    float* framedata;

    // Placeholder buffer used for prefixing data at recipient.
/*
    uint32_t start;
    uint32_t packet_length;
    uint8_t packet_crc;
    uint8_t spr;
    uint8_t sprd;
    uint32_t contentid;
    uint32_t info;
    uint32_t data_length;
*/
    uint8_t mcp_placeholder[3+5*4];

} XepDispatchMessageContentRadardataFramePacket_t;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Dispatch message to hostcom asking to send ack to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_ack(XepDispatch_t* dispatch);

/**
 * @brief Dispatch message to hostcom asking to send error to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_error(XepDispatch_t* dispatch, uint32_t errorcode);

/**
 * @brief Dispatch message to hostcom asking to send pong to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_pong(XepDispatch_t* dispatch, uint32_t pongval);

/**
 * @brief Dispatch message to hostcom asking to send system message to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_system(XepDispatch_t* dispatch, uint32_t contentid);

/**
 * @brief Dispatch message to hostcom asking to send radar frame data to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_forward_radardata_frame_packet(XepDispatch_t* dispatch, XepDispatchMessage_t* message);

/**
 * @brief Dispatch message to hostcom asking to send string to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_data_stringn(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, unsigned char* data, uint32_t length);

/**
 * @brief Dispatch message to hostcom asking to send string to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_data_string(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, char* format, ...)__attribute__((format(gnu_printf, 4, 5)));

/**
 * @brief Dispatch message to hostcom asking to send byte array to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_data_byte(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, uint8_t* data, uint32_t length);

/**
 * @brief Dispatch message to hostcom asking to send float array to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_data_float(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, float* data, uint32_t length);

/**
 * @brief Dispatch message to hostcom asking to reply with int array to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_reply_int(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, int32_t* data, uint32_t length);

/**
 * @brief Dispatch message to hostcom asking to reply with byte array to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_reply_byte(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, uint8_t* data, uint32_t length);

/**
 * @brief Dispatch message to hostcom asking to reply with string to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_reply_string(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, char* format, ...)__attribute__((format(gnu_printf, 4, 5)));

/**
 * @brief Dispatch message to hostcom asking to reply with string to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_reply_stringn(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, unsigned char* data, uint32_t length);

/**
 * @brief Dispatch message to hostcom asking to reply with float to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_reply_float(XepDispatch_t* dispatch, uint32_t contentid, uint32_t info, float* data, uint32_t length);

/**
 * @brief Dispatch message to hostcom asking to send profile data sleep to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_appdata_sleep(XepDispatch_t* dispatch, uint32_t counter, uint32_t state_code, float state_data, float distance, uint32_t signal_quality, float movement_slow, float movement_fast);

/**
 * @brief Dispatch message to hostcom asking to send profile data respiration to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_appdata_respiration(XepDispatch_t* dispatch, uint32_t counter, uint32_t state_code, uint32_t state_data, float distance, float movement, uint32_t signal_quality);

/**
 * @brief Dispatch message to hostcom asking to send profile data truepresence single to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_appdata_truepresence_single(XepDispatch_t* dispatch, uint32_t counter, uint32_t presence_state, float distance, uint8_t direction, uint32_t signal_quality);

/**
 * @brief Dispatch message to hostcom asking to send profile data truepresence movinglist to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_appdata_truepresence_movinglist(XepDispatch_t* dispatch, uint32_t counter, uint32_t presence_state, uint32_t movement_interval_count, uint32_t detection_count, const float *movement_slow_item, const float *movement_fast_item, const float *detection_distance, const float *detection_radar_cross_section, const float *detection_velocity);
//uint32_t dispatch_message_hostcom_send_appdata_truepresence_movinglist(XepDispatch_t* dispatch, uint32_t counter, uint32_t presence_state, uint32_t movement_interval_count, uint32_t detection_count, float *movement_slow_item, float *movement_fast_item, float *detection_distance, float *detection_radar_cross_section, float *detection_velocity);

/**
 * @brief Dispatch message to hostcom asking to send profile data baseband amplitude/phase to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_appdata_baseband_amplitude_phase(XepDispatch_t* dispatch, uint32_t counter, uint32_t num_of_bins, float bin_length, float sampling_frequency, float carrier_frequency, float range_offset, const float *data_amplitude, const float *data_phase);

/**
 * @brief Dispatch message to hostcom asking to send profile data baseband IQ to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_appdata_baseband_iq(XepDispatch_t* dispatch, uint32_t counter, uint32_t num_of_bins, float bin_length, float sampling_frequency, float carrier_frequency, float range_offset, const float *signal_i, const float *signal_q);

/**
 * @brief Dispatch message to hostcom asking to send profile parameter file to host.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_hostcom_send_appdata_profileparameterfile(XepDispatch_t* dispatch, uint32_t filename_length, uint32_t data_length, const char *filename, const char *data);

/**
 * @brief Prepare for sending radardata frame.
 *
 * @return Status of execution
 */
uint32_t dispatch_message_radardata_prepare_frame(XepDispatchMessageContentRadardataFramePacket_t** frame_packet, MemoryBlock_t** memoryblock, XepDispatch_t* dispatch, uint32_t bin_count);

#ifdef __cplusplus
}
#endif



#endif // XEP_DISPATCH_MESSAGES_H
