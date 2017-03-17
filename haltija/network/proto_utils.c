#include "proto_utils.h"

bool decode_repeated_doubles(pb_istream_t *stream, const pb_field_t *field, void **arg) {
 
    BufferInfo_t * info = (BufferInfo_t *) *arg;
    
    double * dbuf = (double *)info->buffer;

    while(stream->bytes_left > 0) {
        
        if (info->pos + 8 > info->buf_size_bytes) {
            return false;
        }
        
        pb_decode_fixed64(stream, dbuf);
        dbuf += 1;
        info->pos += sizeof(double);
    }
    
    return true;
    
}

bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    BufferInfo_t * info = (BufferInfo_t *) *arg;
    
    if (info->buf_size_bytes < stream->bytes_left) {
        return false;
    }
    
    if (!pb_read(stream,info->buffer,stream->bytes_left)) {
        return false;
    }
    
    return true;
}

bool decode_repeated_bytes_fields(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    /*
     //write string tag for delimited field
     if (!pb_encode_tag(stream, PB_WT_STRING, field->tag)) {
     return false;
     }
     
     //write size
     if (!pb_encode_varint(stream, (uint64_t)bytes_read)) {
     return false;
     }
     
     //write buffer
     if (!pb_write(stream, buffer, bytes_read)) {
     return false;
     }
     */
    
    BufferInfo_t * info = (BufferInfo_t *) *arg;
    
    if (!info) {
        return false;
    }
    
    if (!info->buffer) {
        return false;
    }
    
    
    uint8_t * buf = info->buffer + info->pos;
    
    if (info->pos + stream->bytes_left > info->buf_size_bytes) {
        return false;
    }
    
    info->pos += stream->bytes_left;
    
    //TODO do a memcpy instead of byte by byte
    if (!pb_read(stream,buf,stream->bytes_left)) {
        return false;
    }
    
    return true;
}


bool encode_buffer_fields(pb_ostream_t * stream, const pb_field_t * field, void * const * arg) {
    BufferInfo_t * info = *arg;
    
    //write string tag for delimited field
    if (!pb_encode_tag(stream, PB_WT_STRING, field->tag)) {
        return false;
    }
    
    //write size
    if (!pb_encode_varint(stream, info->buf_size_bytes)) {
        return false;
    }
    
    //write buffer
    if (!pb_write(stream, info->buffer, info->buf_size_bytes)) {
        return false;
    }
    
    
    return true;
    
}


bool encode_string_fields(pb_ostream_t * stream, const pb_field_t *field, void * const *arg) {
    const char * str = *arg;
    
    if(!str) {
        //LOGI("_encode_string_fields: No string to encode\n");
        return false;
    }
    
    //write tag
    //if (!pb_encode_tag(stream, PB_WT_STRING, field->tag)) { // Not sure should do this,
    // This is for encoding byte array
    if (!pb_encode_tag_for_field(stream, field)){
        return 0;
    }
    
    return pb_encode_string(stream, (uint8_t*)str, strlen(str));
}
