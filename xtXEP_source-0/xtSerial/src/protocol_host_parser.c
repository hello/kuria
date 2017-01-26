#include "protocol_host_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>


static uint32_t bytes_to_uint32(const unsigned char * data)
{
    return *(const uint32_t*)(data);
}

static float bytes_to_float(const unsigned char * data)
{
    return *(const float*)(data);
}


static void default_pong_callback(uint32_t pong, void * user_data) {(void)pong; (void)user_data;}
static void default_ack_callback(Ack ack, void* user_data) {(void)ack; (void)user_data;}
static void default_error_callback(uint32_t error, void * user_data) {(void)error; (void)user_data;}
static void default_datafloat_callback(FloatData fd, void * user_data) {(void)fd; (void)user_data;}
static void default_reply_callback(Reply reply, void * user_data) {(void)reply; (void)user_data;}
static void default_respiration_callback(RespirationData rd, void * user_data){(void)rd; (void)user_data;}
static void default_sleep_callback(SleepData rd, void * user_data){(void)rd; (void)user_data;}
static void default_baseband_ap_callback(BasebandApData bap, void * user_data){(void)bap; (void)user_data;}
static void default_baseband_iq_callback(BasebandIqData bap, void * user_data){(void)bap; (void)user_data;}
static void default_true_presence_single(TruepresenceSingleDataType tps, void * user_data){(void)tps; (void)user_data;}
static void default_true_presence_movinglist(TruepresenceMovingListDataType tpml, void * user_data){(void)tpml; (void)user_data;}


void init_host_parser(HostParser * parser)
{
    parser->pong = default_pong_callback;
    parser->ack = default_ack_callback;
    parser->error = default_error_callback;
    parser->reply = default_reply_callback;
    parser->data_float = default_datafloat_callback;
    parser->respiration = default_respiration_callback;
    parser->sleep = default_sleep_callback;
    parser->baseband_ap = default_baseband_ap_callback;
    parser->baseband_iq = default_baseband_iq_callback;
    parser->true_presence_single = default_true_presence_single;
    parser->true_presence_movinglist = default_true_presence_movinglist;
}


int parse_error(
    HostParser * parser,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        parser->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

     if(data[0] != XTS_SPR_ERROR)
         parser->error(UNINITILIZED_ERROR_CODE, user_data);

     const unsigned int offset = 1;
     const uint32_t error_code = bytes_to_uint32(&data[offset]);
     parser->error(error_code, user_data);
     return error_code;
}


int parse_data_float(
    HostParser * p,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        p->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    if (data[0] != XTS_SPR_DATA ||
        data[1] != XTS_SPRD_FLOAT) {
        return parse_error(p, data, length, user_data);
    }

    const unsigned int header_size = 14;

    if(length < header_size) {
        p->error(UNINITILIZED_ERROR_CODE, user_data);
        return 1;
    }

    FloatData fd;
    unsigned int offset = 2;
    fd.content_id = *(const uint32_t *)(&data[offset]);
    offset += sizeof(fd.content_id);
    fd.info = *(const uint32_t *)(&data[offset]);
    const unsigned int data_offset = header_size;
    const unsigned int data_size = length - header_size;
    fd.length = data_size/sizeof(float);
    fd.data = (float*)&data[data_offset];
    p->data_float(fd, user_data);
    return 0;
}


int parse_ack(
    HostParser * parser,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        parser->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    if (data[0] != XTS_SPR_ACK) {
        return parse_error(parser, data, length, user_data);
    }
    Ack ack = {0};
    parser->ack(ack, user_data);
    return 0;
}

int parse_reply(
    HostParser * p,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        p->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    unsigned int offset = 0;
    if (data[offset] != XTS_SPR_REPLY) {
        return parse_error(p, data, length, user_data);
    }
    offset += 1;
    Reply reply;
    reply.data_type = (serial_protocol_response_datatype_t)data[offset];
    offset += 1;
    reply.content_id = bytes_to_uint32(&data[offset]);
    offset += sizeof(uint32_t);
    reply.info = bytes_to_uint32(&data[offset]);
    offset += sizeof(uint32_t);

    if (offset >= length) {
        reply.length = 0;
        p->reply(reply, user_data);
        return 0;
    }

    reply.length = bytes_to_uint32(&data[offset]);
    offset += sizeof(uint32_t);
    reply.data = (uint8_t*)&data[offset];
    offset += reply.length;
    reply.data_size = (uint8_t)data[offset];
    p->reply(reply, user_data);
    return 0;
}


int parse_pong(
    HostParser * p,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        p->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    if (data[0] != XTS_SPR_PONG) {
        return parse_error(p, data, length, user_data);
    }

    if (length < 1 + sizeof(uint32_t)) {
        p->error(UNINITILIZED_ERROR_CODE, user_data);
        return 1;
    }
    const unsigned int offset = 1;
    const uint32_t pongval = bytes_to_uint32(&data[offset]);
    p->pong(pongval, user_data);
    return PARSE_OK;
}


int parse_data(
    HostParser * p,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        p->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    const unsigned int offset = 1;
    if(data[offset] == XTS_SPRD_FLOAT) {
        return parse_data_float(p, data, length, user_data);
    }
    return ERROR;
}




int parse_true_presence_movinglist(
    HostParser * parser,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        parser->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    unsigned int offset = 0;
    if(data[offset] != XTS_SPR_APPDATA) {
        parser->error(ERROR_WRONG_CONTENT, user_data);
        return ERROR_WRONG_CONTENT;
    }

    offset = 1;
    const uint32_t appdata_id = bytes_to_uint32(&data[offset]);
    offset += sizeof(appdata_id);
    if(appdata_id != XTS_ID_TRUEPRESENCE_MOVINGLIST) {
        parser->error(ERROR_WRONG_CONTENT, user_data);
        return ERROR_WRONG_CONTENT;
    }

    TruepresenceMovingListDataType true_presence;
    true_presence.frame_counter = bytes_to_uint32(&data[offset]);
    offset += sizeof(true_presence.frame_counter);

    true_presence.state = bytes_to_uint32(&data[offset]);
    offset += sizeof(true_presence.state);

    true_presence.interval_count = bytes_to_uint32(&data[offset]);
    offset += sizeof(true_presence.interval_count);

    true_presence.detection_count = bytes_to_uint32(&data[offset]);
    offset += sizeof(true_presence.detection_count);

    true_presence.movement_slow = (float*)&data[offset];
    offset += true_presence.interval_count * sizeof(*true_presence.movement_slow);

    true_presence.movement_fast = (float*)&data[offset];
    offset += true_presence.interval_count * sizeof(*true_presence.movement_slow);

    true_presence.distance = (float*)&data[offset];
    offset += true_presence.detection_count * sizeof(*true_presence.distance);

    true_presence.radar_cross_section = (float*)&data[offset];
    offset += true_presence.detection_count * sizeof(*true_presence.radar_cross_section);

    true_presence.velocity = (float*)&data[offset];
    offset += true_presence.detection_count * sizeof(*true_presence.velocity);

    parser->true_presence_movinglist(true_presence, user_data);
    return 0;
}


int parse_true_presence_single(
    HostParser * parser,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        parser->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    unsigned int offset = 0;
    if(data[offset] != XTS_SPR_APPDATA) {
        parser->error(ERROR_WRONG_CONTENT, user_data);
        return ERROR_WRONG_CONTENT;
    }

    offset = 1;
    const uint32_t appdata_id = bytes_to_uint32(&data[offset]);
    offset += sizeof(appdata_id);
    if(appdata_id != XTS_ID_TRUEPRESENCE_SINGLE) {
        parser->error(ERROR_WRONG_CONTENT, user_data);
        return ERROR_WRONG_CONTENT;
    }

    TruepresenceSingleDataType true_presence;
    true_presence.frame_counter = bytes_to_uint32(&data[offset]);
    offset += sizeof(true_presence.frame_counter);

    true_presence.presence_state = bytes_to_uint32(&data[offset]);
    offset += sizeof(true_presence.presence_state);

    true_presence.distance = bytes_to_float(&data[offset]);
    offset += sizeof(true_presence.distance);

    true_presence.direction = data[offset];
    offset += sizeof(true_presence.direction);

    true_presence.signal_quality = bytes_to_uint32(&data[offset]);
    parser->true_presence_single(true_presence, user_data);
    return PARSE_OK;
}


int parse_baseband_iq(
    HostParser * parser,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        parser->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    unsigned int offset = 0;
    if(data[offset] != XTS_SPR_APPDATA) {
        parser->error(ERROR_WRONG_CONTENT, user_data);
        return ERROR_WRONG_CONTENT;
    }

    offset = 1;
    const uint32_t appdata_id = bytes_to_uint32(&data[offset]);
    if(appdata_id != XTS_ID_BASEBAND_IQ) {
        parser->error(ERROR_WRONG_CONTENT, user_data);
        return ERROR_WRONG_CONTENT;
    }

    if (length < sizeof(BasebandApData)) {
        parser->error(ERROR_SHORT_DATA, user_data);
        return ERROR_SHORT_DATA;
    }

    BasebandIqData bap;
    offset += sizeof(appdata_id);
    bap.frame_counter = bytes_to_uint32(&data[offset]);
    offset += sizeof(bap.frame_counter);
    bap.num_bins = bytes_to_uint32(&data[offset]);
    offset += sizeof(bap.num_bins);
    bap.bin_length = bytes_to_float(&data[offset]);
    offset += sizeof(bap.bin_length);
    bap.sample_frequency = bytes_to_float(&data[offset]);
    offset += sizeof(bap.sample_frequency);
    bap.carrier_frequency = bytes_to_float(&data[offset]);
    offset += sizeof(bap.carrier_frequency);
    bap.range_offset = bytes_to_float(&data[offset]);
    offset += sizeof(bap.range_offset);
    bap.i_data = (const float *)(&data[offset]);
    offset += sizeof(float) * bap.num_bins;
    bap.q_data = (const float *)(&data[offset]);

    parser->baseband_iq(bap, user_data);
    return PARSE_OK;
}


int parse_baseband_ap(
    HostParser * parser,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        parser->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    unsigned int offset = 0;
    if (data[offset] != XTS_SPR_APPDATA) {
        parser->error(ERROR_WRONG_CONTENT, user_data);
        return ERROR_WRONG_CONTENT;
    }

    offset = 1;
    const uint32_t appdata_id = bytes_to_uint32(&data[offset]);
    if (appdata_id != XTS_ID_BASEBAND_AMPLITUDE_PHASE) {
        parser->error(ERROR_WRONG_CONTENT, user_data);
        return ERROR_WRONG_CONTENT;
    }

    if (length < sizeof(BasebandApData)) {
        parser->error(ERROR_SHORT_DATA, user_data);
        return ERROR_SHORT_DATA;
    }

    BasebandApData bap;
    offset += sizeof(appdata_id);
    bap.frame_counter = bytes_to_uint32(&data[offset]);
    offset += sizeof(bap.frame_counter);
    bap.num_bins = bytes_to_uint32(&data[offset]);
    offset += sizeof(bap.num_bins);
    bap.bin_length = bytes_to_float(&data[offset]);
    offset += sizeof(bap.bin_length);
    bap.sample_frequency = bytes_to_float(&data[offset]);
    offset += sizeof(bap.sample_frequency);
    bap.carrier_frequency = bytes_to_float(&data[offset]);
    offset += sizeof(bap.carrier_frequency);
    bap.range_offset = bytes_to_float(&data[offset]);
    offset += sizeof(bap.range_offset);
    bap.amplitude = (const float *)(&data[offset]);
    offset += sizeof(float) * bap.num_bins;
    bap.phase = (const float *)(&data[offset]);

    parser->baseband_ap(bap, user_data);
    return PARSE_OK;
}



int parse_sleep_status(
    HostParser * parser,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        parser->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    unsigned int offset = 1;
    const uint32_t appdata_id = bytes_to_uint32(&data[offset]);
    if(appdata_id != XTS_ID_SLEEP_STATUS) {
        parser->error(ERROR_WRONG_CONTENT, user_data);
        return 1;
    }
    offset = 1 + 4;
    if (length < sizeof(RespirationData)) {
        parser->error(ERROR_SHORT_DATA, user_data);
        return 1;
    }

    SleepData sd;
    sd.frame_counter = bytes_to_uint32(&data[offset]);
    offset += sizeof(sd.frame_counter);
    sd.sensor_state = bytes_to_uint32(&data[offset]);
    offset += sizeof(sd.sensor_state);
    sd.respiration_rate = bytes_to_float(&data[offset]);
    offset += sizeof(sd.respiration_rate);
    sd.distance = bytes_to_float(&data[offset]);
    offset += sizeof(sd.distance);
    sd.signal_quality = bytes_to_uint32(&data[offset]);
    offset += sizeof(sd.signal_quality);
    sd.movement_slow = bytes_to_float(&data[offset]);
    offset += sizeof(sd.movement_slow);
    sd.movement_fast = bytes_to_float(&data[offset]);
    offset += sizeof(sd.movement_fast);
    parser->sleep(sd, user_data);
    return 0;
}


int parse_respiration_status(
    HostParser * parser,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        parser->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    unsigned int offset = 1;
    const uint32_t appdata_id = bytes_to_uint32(&data[offset]);
    if(appdata_id != XTS_ID_RESP_STATUS) {
        parser->error(ERROR_WRONG_CONTENT, user_data);
        return 1;
    }
    offset = 1 + 4;
    if (length < sizeof(RespirationData)) {
        parser->error(ERROR_SHORT_DATA, user_data);
        return 1;
    }

    RespirationData rd;
    rd.frame_counter = bytes_to_uint32(&data[offset]);
    offset += sizeof(rd.frame_counter);
    rd.sensor_state = bytes_to_uint32(&data[offset]);
    offset += sizeof(rd.sensor_state);
    rd.respiration_rate = bytes_to_uint32(&data[offset]);
    offset += sizeof(rd.respiration_rate);
    rd.distance = bytes_to_float(&data[offset]);
    offset += sizeof(rd.distance);
    rd.movement = bytes_to_float(&data[offset]);
    offset += sizeof(rd.movement);
    rd.signal_quality = bytes_to_uint32(&data[offset]);
    parser->respiration(rd, user_data);
    return 0;
}


int parse_appdata(
    HostParser * parser,
    const unsigned char * data,
    unsigned int length,
    void * user_data)
{
    if (!length) {
        parser->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    const unsigned int offset = 1;
    const uint32_t appdata_id = bytes_to_uint32(&data[offset]);

    if(appdata_id == XTS_ID_RESP_STATUS) {
        return parse_respiration_status(parser, data, length, user_data);
    }

    if(appdata_id == XTS_ID_SLEEP_STATUS) {
        return parse_sleep_status(parser, data, length, user_data);
    }

    if(appdata_id == XTS_ID_BASEBAND_AMPLITUDE_PHASE) {
        return parse_baseband_ap(parser, data, length, user_data);
    }

    if(appdata_id == XTS_ID_BASEBAND_IQ) {
        return parse_baseband_iq(parser, data, length, user_data);
    }

    if(appdata_id == XTS_ID_TRUEPRESENCE_SINGLE) {
        return parse_true_presence_single(parser, data, length, user_data);
    }
    if(appdata_id == XTS_ID_TRUEPRESENCE_MOVINGLIST) {
        return parse_true_presence_movinglist(parser, data, length, user_data);
    }

    return PARSE_OK;
}


int parse(HostParser * parser, const unsigned char * data, unsigned int length, void * user_data)
{
    if (!length) {
        parser->error(ERROR_NO_DATA, user_data);
        return ERROR_NO_DATA;
    }

    const unsigned int offset = 0;
    if (data[offset] == XTS_SPR_ERROR) {
        return parse_error(parser, data, length, user_data);
    }

    if(data[offset] == XTS_SPR_ACK) {
        return parse_ack(parser, data, length, user_data);
    }
    else if (data[offset] == XTS_SPR_PONG) {
        return parse_pong(parser, data, length, user_data);
    }
    else if (data[offset] == XTS_SPR_DATA) {
        return parse_data(parser, data, length, user_data);
    }
    else if (data[offset] == XTS_SPR_APPDATA) {
        return parse_appdata(parser, data, length, user_data);
    }
    else {
        parser->error(2,user_data);
    }

    return ERROR_UNKNOWN_PACKET;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */


