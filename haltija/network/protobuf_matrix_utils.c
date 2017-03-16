#include "protobuf_matrix_utils.h"
#include "simple_matrix.pb.h"
#include <stdbool.h>
#include "pb_encode.h"
#include "pb_decode.h"

#include <string.h>
#include <stdlib.h>

const static SimpleMatrixDataType _type_map[] =
{
    SimpleMatrixDataType_SINT8,
    SimpleMatrixDataType_SINT16,
    SimpleMatrixDataType_SINT32,
    SimpleMatrixDataType_SINT64,
    SimpleMatrixDataType_FLOAT32,
    SimpleMatrixDataType_FLOAT64,
    SimpleMatrixDataType_COMPLEXFLOAT64,
    SimpleMatrixDataType_COMPLEXFLOAT128
};

const static size_t _mat_scalar_sizes[] = {
    1,
    2,
    4,
    8,
    4,
    8,
    8,
    16
};


typedef struct {
    unsigned char * buffer;
    size_t buf_size_bytes;
} BufferInfo_t;

static bool decode_repeated_bytes_fields(pb_istream_t *stream, const pb_field_t *field, void **arg) {
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
    
    while (1) {
        uint32_t tag;
        bool eof;
        pb_wire_type_t wire_type;
        
        if (!pb_decode_tag(stream, &wire_type, &tag, &eof)) {
            if (eof) {
                break;
            }
            
            return false;
        }
        
        uint64_t bytes_read = 0;
        
        if (!pb_decode_varint(stream,&bytes_read)) {
            return false;
        }
        
        //TODO decide if I malloc every time or just concatenate
        //leaning towards concatenate and have a max buffer size
        
        /*
        if (!pb_read(stream,buf,bytes_read)) {
            return false;
        }
         */
        
    }
    
    return true;
}

static bool encode_buffer_fields(pb_ostream_t * stream, const pb_field_t * field, void * const * arg) {
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


static bool encode_string_fields(pb_ostream_t * stream, const pb_field_t *field, void * const *arg) {
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


EEncodeStatus_t protobuf_matrix_utils_create_and_write_protobuf(void * buffer,size_t * bytes_written,size_t buffer_size,const char * name, const char * device_id,const size_t num_rows, const size_t num_cols, const void * data, EMatrixDataScalarType_t mat_scalar_type, Timestamp_t timestamp_utc_millis, int tzoffset_millis) {
        
    SimpleMatrix mat;
    const size_t scalar_size = _mat_scalar_sizes[mat_scalar_type];
    const size_t num_buf_bytes = scalar_size * num_rows * num_cols;

    memset(&mat,0,sizeof(mat));

    //bounds check on the enum
    if ((int)mat_scalar_type >= sizeof(_type_map) / sizeof(_type_map[0]) || (int)mat_scalar_type < 0) {
        return encode_status_invalid_input;
    }
    
    
    
    mat.has_data_type = true;
    mat.data_type = _type_map[mat_scalar_type];
    
    mat.has_num_cols = true;
    mat.num_cols = num_cols;
    
    mat.has_timestamp_utc_millis = true;
    mat.timestamp_utc_millis = timestamp_utc_millis;
    
    mat.has_tz_offset_millis = true;
    mat.tz_offset_millis = tzoffset_millis;
    
    if (name) {
        mat.id.funcs.encode = encode_string_fields;
        mat.id.arg = (void *)name;
    }
    
    if (device_id) {
        mat.device_id.funcs.encode = encode_string_fields;
        mat.device_id.arg = (void *)device_id;
    }
    
    const BufferInfo_t info = {(void *)data,num_buf_bytes};
    
    mat.payload.funcs.encode = encode_buffer_fields;
    mat.payload.arg = (void *)&info;
    
    pb_ostream_t bufstream = pb_ostream_from_buffer(buffer,buffer_size);
    
    if (!pb_encode(&bufstream, SimpleMatrix_fields, &mat)) {
        return encode_status_buffer_too_small;
    }
    
    *bytes_written = bufstream.bytes_written;
    
    return encode_status_success;
}

bool protobuf_matrix_utils_decode_protobuf(const void * bytes, const size_t num_bytes, DecodedSimpleMatrix_t * decoded_mat) {
    /*
    pb_istream_t istream = pb_istream_from_buffer(bytes, num_bytes);
    
    memset(&mat,0,sizeof(mat));

    mat.device_id.funcs = pb_decode_
    
    pb_decode(&istream,SimpleMatrix_fields,&mat);
    */
    

    return false;
}


