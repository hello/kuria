#ifndef PROTOCOL_TARGET_H
#define PROTOCOL_TARGET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "protocol.h"

int createAckCommand(
    AppendCallback callback,
    void * user_data);

int createErrorCommand(
	int errorcode,
	AppendCallback callback,
	void * user_data);

int createPongCommand(
	int pongval,
	AppendCallback callback,
	void * user_data);

int createSystemCommand(
	uint32_t contentid,
	uint8_t* data,
	uint32_t length,
	AppendCallback callback,
	void * user_data);

int createAppdataSleepCommand(
    uint32_t counter,
    uint32_t stateCode,
    float stateData,
    float distance,
    uint32_t signalQuality,
    float movementSlow,
    float movementFast,
    AppendCallback callback,
    void * user_data);

int createAppdataRespirationCommand(
    uint32_t counter,
    uint32_t stateCode,
    uint32_t stateData,
    float distance,
    float movement,
    uint32_t signalQuality,
    AppendCallback callback,
    void * user_data);

int createAppdataTruePresenceSingleCommand(
    uint32_t counter,
    uint32_t presenceState,
    float distance,
    uint8_t direction,
    uint32_t signalQuality,
    AppendCallback callback,
    void * user_data);

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
    void * user_data);

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
    void * user_data);

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
    void * user_data);

int createAppdataProfileParameterFileCommand(
    uint32_t filename_length,
    uint32_t data_length,
    unsigned char * filename,
    unsigned char * data,
    AppendCallback callback,
    void * user_data);

int createDataFloatCommand(
	uint32_t contentid,
	uint32_t info,
	float* data,
	uint32_t length,
	AppendCallback callback,
	void * user_data);

int createDataByteCommand(
    uint32_t contentid,
    uint32_t info,
    uint8_t* data,
    uint32_t length,
    AppendCallback callback,
    void * user_data);

int createDataStringnCommand(
	uint32_t contentid,
	uint32_t info,
	unsigned char* data,
	uint32_t length,
	AppendCallback callback,
	void * user_data);

int createDataUserCommand(
	uint32_t contentid,
	uint32_t info,
	void* data,
	uint32_t length,
	AppendCallback callback,
	void * user_data);

int createReplyByteCommand(
	uint32_t contentid,
	uint32_t info,
	uint8_t* data,
	uint32_t length,
	AppendCallback callback,
	void * user_data);

int createReplyIntCommand(
    uint32_t contentid,
    uint32_t info,
    int32_t* data,
    uint32_t length,
    AppendCallback callback,
    void * user_data);

int createReplyStringnCommand(
    uint32_t contentid,
    uint32_t info,
    unsigned char* data,
    uint32_t length,
    AppendCallback callback,
    void * user_data);

int createReplyFloatCommand(
    uint32_t contentid,
    uint32_t info,
    float* data,
    uint32_t length,
    AppendCallback callback,
    void * user_data);

int createHilUpCommand(
	serial_protocol_response_datatype_t datatype, 
	serial_protocol_hil_t hil_command, 
	uint32_t info, 
	uint32_t length, 
	uint8_t* data, 
	uint8_t datasize,
	AppendCallback callback,
	void * user_data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PROTOCOL_TARGET_H */
