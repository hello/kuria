#include "protocol_target.h"
#include "protocol_helpers.h"
#include <stdlib.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



int createAckCommand(
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_ACK, callback, user_data);
    packet_end(callback, user_data);
    return 0;
}

int createErrorCommand(
    int errorcode,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_ERROR, callback, user_data);
    process_int(errorcode, callback, user_data);
    packet_end(callback, user_data);
    return 0;
}

int createPongCommand(
    int pongval,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_PONG, callback, user_data);
    process_int(pongval, callback, user_data);
    packet_end(callback, user_data);
    return 0;
}

int createSystemCommand(
    uint32_t contentid,
    uint8_t* data,
    uint32_t length,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_SYSTEM, callback, user_data);
    process_int(contentid, callback, user_data);
    process_bytes(data, length, callback, user_data);
    packet_end(callback, user_data);
    return 0;
}

int createAppdataSleepCommand(
    uint32_t counter,
    uint32_t stateCode,
    float stateData,
    float distance,
    uint32_t signalQuality,
    float movementSlow,
    float movementFast,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_APPDATA, callback, user_data);
    process_int(XTS_ID_SLEEP_STATUS, callback, user_data);
    process_int(counter, callback, user_data);
    process_int(stateCode, callback, user_data);
    process_float(stateData, callback, user_data);
    process_float(distance, callback, user_data);
    process_int(signalQuality, callback, user_data);
    process_float(movementSlow, callback, user_data);
    process_float(movementFast, callback, user_data);
    packet_end(callback, user_data);
    return 0;
}

int createAppdataRespirationCommand(
    uint32_t counter,
    uint32_t stateCode,
    uint32_t stateData,
    float distance,
    float movement,
    uint32_t signalQuality,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_APPDATA, callback, user_data);
    process_int(XTS_ID_RESP_STATUS, callback, user_data);
    process_int(counter, callback, user_data);
    process_int(stateCode, callback, user_data);
    process_int(stateData, callback, user_data);
    process_float(distance, callback, user_data);
    process_float(movement, callback, user_data);
    process_int(signalQuality, callback, user_data);
    packet_end(callback, user_data);
    return 0;
}

int createAppdataTruePresenceSingleCommand(
    uint32_t counter,
    uint32_t presenceState,
    float distance,
    uint8_t direction,
    uint32_t signalQuality,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_APPDATA, callback, user_data);
    process_int(XTS_ID_TRUEPRESENCE_SINGLE, callback, user_data);
    process_int(counter, callback, user_data);
    process_int(presenceState, callback, user_data);
    process_float(distance, callback, user_data);
    process_byte(direction, callback, user_data);
    process_int(signalQuality, callback, user_data);
    packet_end(callback, user_data);
    return 0;
}

int createAppdataTruePresenceMovinglistCommand(
    uint32_t counter,
    uint32_t presenceState,
    uint32_t movementIntervalCount,
    uint32_t detectionCount,
    float* movementSlowItem,
    float* movementFastItem,
    float* detectionDistance,
    float* detectionRadarCrossSection,
    float* detectionVelocity,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_APPDATA, callback, user_data);
    process_int(XTS_ID_TRUEPRESENCE_MOVINGLIST, callback, user_data);
    process_int(counter, callback, user_data);
    process_int(presenceState, callback, user_data);
    process_int(movementIntervalCount, callback, user_data);
    process_int(detectionCount, callback, user_data);
    process_floats(movementSlowItem, movementIntervalCount, callback, user_data);
    process_floats(movementFastItem, movementIntervalCount, callback, user_data);
    process_floats(detectionDistance, detectionCount, callback, user_data);
    process_floats(detectionRadarCrossSection, detectionCount, callback, user_data);
    process_floats(detectionVelocity, detectionCount, callback, user_data);
    packet_end(callback, user_data);
    return 0;
}

int createAppdataBasebandAmplitudePhaseCommand(
    uint32_t counter,
    uint32_t numOfBins,
    float binLength,
    float samplingFrequency,
    float carrierFrequency,
    float rangeOffset,
    float* dataAmplitude,
    float* dataPhase,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_APPDATA, callback, user_data);
    process_int(XTS_ID_BASEBAND_AMPLITUDE_PHASE, callback, user_data);
    process_int(counter, callback, user_data);
    process_int(numOfBins, callback, user_data);
    process_float(binLength, callback, user_data);
    process_float(samplingFrequency, callback, user_data);
    process_float(carrierFrequency, callback, user_data);
    process_float(rangeOffset, callback, user_data);
    process_floats(dataAmplitude, numOfBins, callback, user_data);
    process_floats(dataPhase, numOfBins, callback, user_data);
    packet_end(callback, user_data);
    return 0;
}

int createAppdataBasebandIQCommand(
    uint32_t counter,
    uint32_t numOfBins,
    float binLength,
    float samplingFrequency,
    float carrierFrequency,
    float rangeOffset,
    float* signalI,
    float* signalQ,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_APPDATA, callback, user_data);
    process_int(XTS_ID_BASEBAND_IQ, callback, user_data);
    process_int(counter, callback, user_data);
    process_int(numOfBins, callback, user_data);
    process_float(binLength, callback, user_data);
    process_float(samplingFrequency, callback, user_data);
    process_float(carrierFrequency, callback, user_data);
    process_float(rangeOffset, callback, user_data);
    process_floats(signalI, numOfBins, callback, user_data);
    process_floats(signalQ, numOfBins, callback, user_data);
    packet_end(callback, user_data);
    return 0;
}

int createAppdataProfileParameterFileCommand(
    uint32_t filename_length,
    uint32_t data_length,
    unsigned char * filename,
    unsigned char * data,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_APPDATA, callback, user_data);
    process_int(XTS_ID_PROFILE_PARAMETERFILE, callback, user_data);
    process_int(filename_length, callback, user_data);
    process_int(data_length, callback, user_data);
    process_bytes(filename, filename_length, callback, user_data);
    process_bytes(data, data_length, callback, user_data);
    packet_end(callback, user_data);
    return 0;
}

int createDataFloatCommand(
    uint32_t contentid,
    uint32_t info,
    float* data,
    uint32_t length,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_DATA, callback, user_data);
    process_byte(XTS_SPRD_FLOAT, callback, user_data);
    process_int(contentid, callback, user_data);
    process_int(info, callback, user_data);
    if (length>0)
    {
        process_int(length, callback, user_data);
        process_floats(data, length, callback, user_data);
    }
    packet_end(callback, user_data);
    return 0;
}

int createDataByteCommand(
    uint32_t contentid,
    uint32_t info,
    uint8_t* data,
    uint32_t length,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_DATA, callback, user_data);
    process_byte(XTS_SPRD_BYTE, callback, user_data);
    process_int(contentid, callback, user_data);
    process_int(info, callback, user_data);
    if (length>0)
    {
        process_int(length, callback, user_data);
        process_bytes(data, length, callback, user_data);
    }
    packet_end(callback, user_data);
    return 0;
}

int createDataStringnCommand(
    uint32_t contentid,
    uint32_t info,
    unsigned char* data,
    uint32_t length,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_DATA, callback, user_data);
    process_byte(XTS_SPRD_STRING, callback, user_data);
    process_int(contentid, callback, user_data);
    process_int(info, callback, user_data);
    if (length>0)
    {
        process_int(length, callback, user_data);
        process_bytes(data, length, callback, user_data);
    }
    packet_end(callback, user_data);
    return 0;
}

int createDataUserCommand(
    uint32_t contentid,
    uint32_t info,
    void* data,
    uint32_t length,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_DATA, callback, user_data);
    process_byte(XTS_SPRD_USER, callback, user_data);
    process_int(contentid, callback, user_data);
    process_int(info, callback, user_data);
    if (length>0)
    {
        process_int(length, callback, user_data);
        process_bytes(data, length, callback, user_data);
    }
    packet_end(callback, user_data);
    return 0;
}

int createReplyIntCommand(
    uint32_t contentid,
    uint32_t info,
    int32_t* data,
    uint32_t length,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_REPLY, callback, user_data);
    process_byte(XTS_SPRD_INT, callback, user_data);
    process_uint(contentid, callback, user_data);
    process_uint(info, callback, user_data);
    if (length>0)
    {
        process_uint(length, callback, user_data);
        process_ints(data, length, callback, user_data);
    }
    packet_end(callback, user_data);
    return 0;
}

int createReplyByteCommand(
	uint32_t contentid,
	uint32_t info,
	uint8_t* data,
	uint32_t length,
	AppendCallback callback,
	void * user_data)
{
	packet_start(callback, user_data);
	process_byte(XTS_SPR_REPLY, callback, user_data);
	process_byte(XTS_SPRD_BYTE, callback, user_data);
	process_uint(contentid, callback, user_data);
	process_uint(info, callback, user_data);
	if (length>0)
	{
		process_uint(length, callback, user_data);
		process_bytes(data, length, callback, user_data);
	}
	packet_end(callback, user_data);
	return 0;
}


int createReplyStringnCommand(
    uint32_t contentid,
    uint32_t info,
    unsigned char* data,
    uint32_t length,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_REPLY, callback, user_data);
    process_byte(XTS_SPRD_STRING, callback, user_data);
    process_uint(contentid, callback, user_data);
    process_uint(info, callback, user_data);
    if (length>0)
    {
        process_uint(length, callback, user_data);
        process_bytes(data, length, callback, user_data);
    }
    packet_end(callback, user_data);
    return 0;
}

int createReplyFloatCommand(
    uint32_t contentid,
    uint32_t info,
    float* data,
    uint32_t length,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_REPLY, callback, user_data);
    process_byte(XTS_SPRD_FLOAT, callback, user_data);
    process_uint(contentid, callback, user_data);
    process_uint(info, callback, user_data);
    if (length>0)
    {
        process_uint(length, callback, user_data);
        process_floats(data, length, callback, user_data);
    }
    packet_end(callback, user_data);
    return 0;
}


int createHilUpCommand(
    serial_protocol_response_datatype_t datatype,
    serial_protocol_hil_t hil_command,
    uint32_t info,
    uint32_t length,
    uint8_t* data,
    uint8_t datasize,
    AppendCallback callback,
    void * user_data)
{
    packet_start(callback, user_data);
    process_byte(XTS_SPR_HIL, callback, user_data);
    process_byte(datatype, callback, user_data);
    process_int((uint32_t)hil_command, callback, user_data);
    process_int(info, callback, user_data);
    if (length>0)
    {
        process_int(length, callback, user_data);
        process_bytes(data, length*datasize, callback, user_data);
    }
    packet_end(callback, user_data);
    return 0;
}



#ifdef __cplusplus
}
#endif /* __cplusplus */


