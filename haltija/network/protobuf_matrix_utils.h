#ifndef _PROTOBUFMATRIXUTILS_H_
#define _PROTOBUFMATRIXUTILS_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
    
    typedef long long Timestamp_t; //TODO move this to shared include
    typedef struct {
        void * buf;
        size_t num_bytes;
    } ByteBuf_t;
    
    typedef enum  {
        mat_sint8,
        mat_sint16,
        mat_sint32,
        mat_sint64,
        mat_float,
        mat_double,
        mat_complex_float,
        mat_complex_double
    } EMatrixDataScalarType_t;
    
    typedef struct {
        EMatrixDataScalarType_t scalar_type;
        void * data;
        size_t num_rows;
        size_t num_cols;
        char id[1024];
        char device_id[1024];
    } DecodedSimpleMatrix_t;
    
    
    /* NOTE THAT THIS ASSUMES THE MATRIX IS ROW MAJOR, WHICH IS NOT THE CASE FOR THE DEFAULT FOR EIGEN  */
    ByteBuf_t protobuf_matrix_utils_create_and_write_protobuf(const char * name, const char * device_id,const size_t num_rows, const size_t num_cols, const void * data, EMatrixDataScalarType_t mat_scalar_type, Timestamp_t timestamp_utc_millis, int tzoffset_millis);
    
    bool protobuf_matrix_utils_decode_protobuf(const void * bytes, const size_t num_bytes, DecodedSimpleMatrix_t * decoded_mat);
    
    

#ifdef __cplusplus
}
#endif

#endif
