#include "gtest/gtest.h"
#include "network/protobuf_matrix_utils.h"
#include <string.h>
#include <stdlib.h>
#include <fstream>

#define GET_TEST_FILE_PATH(x)\
    UNIT_TEST_DATA + std::string(x)

class TestNetwork : public ::testing::Test {
protected:


    virtual void SetUp() {
    }

    virtual void TearDown() {
        
    }
    
    
};

class DISABLED_TestNetwork : public TestNetwork {};

TEST_F(DISABLED_TestNetwork, TestDecodeSimpleMatrix) {

    const std::string filepath = GET_TEST_FILE_PATH("simplematrix.bin");
    char buf[1024] = {0};
    
    std::ifstream fin(filepath, std::ios::in | std::ios::binary );
    fin.read(buf, sizeof(buf));
    
    DecodedSimpleMatrix_t foo;
    protobuf_matrix_utils_decode_protobuf(buf,4242424,&foo);
}

TEST_F(TestNetwork, TestEncodeSimpleMatrix) {
    const double mat[6] = {1,2,3,4,5,6};
    
    uint8_t buf[1024];
    size_t bytes_written = 0;
    
    EEncodeStatus_t status = protobuf_matrix_utils_create_and_write_protobuf(buf,&bytes_written,sizeof(buf),"name", "device_id",3, 2, mat, mat_double, 0LL,0);
    
    ASSERT_EQ(status,encode_status_success);
    
    //TODO decode this
    
    status = protobuf_matrix_utils_create_and_write_protobuf(buf,&bytes_written,bytes_written - 1,"name", "device_id",3, 2, mat, mat_double, 0LL,0);
    
    
    ASSERT_EQ(status,encode_status_buffer_too_small);

    //todo decode

}

