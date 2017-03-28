#include "gtest/gtest.h"
#include "network/protobuf_matrix_utils.h"
#include "network/noveldaprotobuf.h"
#include "network/radarmessageprotobuf.h"
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <cstdlib>
#include <cstring>

#define GET_TEST_FILE_PATH(x)\
    UNIT_TEST_DATA + std::string(x)

class TestNetwork : public ::testing::Test {
protected:


    virtual void SetUp() {
        bytes = NULL;
    }

    virtual void TearDown() {
        if (bytes) {
            free(bytes);
        }
    }
    
    uint8_t * bytes;
};

class DISABLED_TestNetwork : public TestNetwork {};

TEST_F(TestNetwork,TestDecodeNovelda) {
    char buf[100000] = {0};

    const std::string filepath = GET_TEST_FILE_PATH("novelda.bin");
    std::ifstream fin(filepath, std::ios::in | std::ios::binary );
    fin.seekg (0, fin.end);
    int length = fin.tellg();
    fin.seekg (0, fin.beg);
    
    fin.read(buf, sizeof(buf));
    
    NoveldaData_t deserialized_data;
    
    ASSERT_TRUE(NoveldaProtobuf::deserialize_protobuf((uint8_t *)buf, length, deserialized_data));
    ASSERT_EQ(deserialized_data.range_bins.size(),188);
    
}

TEST_F(TestNetwork, TestDecodeSimpleMatrix) {

    const std::string filepath = GET_TEST_FILE_PATH("simplematrix.bin");
    char buf[1024] = {0};
    char bufout[1024] = {0};
    double *f = (double *)bufout;

    std::ifstream fin(filepath, std::ios::in | std::ios::binary );
    fin.read(buf, sizeof(buf));
    
    DecodedSimpleMatrix_t mat;
    memset(&mat,0,sizeof(mat));
    
    protobuf_matrix_utils_decode_protobuf(bufout,sizeof(bufout),&mat,buf,sizeof(buf));
    
    ASSERT_NEAR(f[0],1.0,1e-6);
    ASSERT_NEAR(f[1],3.14159,1e-6);
    ASSERT_NEAR(f[2],2.0,1e-6);
    ASSERT_NEAR(f[3],-42.0,1e-6);
    
    ASSERT_TRUE(mat.num_cols == 2);
    ASSERT_TRUE(mat.num_rows == 2);
    ASSERT_TRUE(mat.scalar_type == SimpleMatrixDataType_FLOAT64);
    ASSERT_TRUE(strcmp(mat.device_id,"device_id") == 0);
    ASSERT_TRUE(strcmp(mat.id,"name") == 0);

}

TEST_F(TestNetwork, TestEncodeSimpleMatrix) {
    const double mat[6] = {1,2,3,4,5,6};
    
    uint8_t buf[1024];
    size_t bytes_written = 0;
    
    EEncodeStatus_t status = protobuf_matrix_utils_create_and_write_protobuf(buf,&bytes_written,sizeof(buf),"name", "device_id",3, 2, mat, SimpleMatrixDataType_FLOAT64, 0LL,0);
    
    ASSERT_EQ(status,encode_status_success);
    
   
    DecodedSimpleMatrix_t dec_mat;
    memset(&dec_mat,0,sizeof(dec_mat));
    char bufout[1024] = {0};
    double *f = (double *)bufout;

    protobuf_matrix_utils_decode_protobuf(bufout,sizeof(bufout),&dec_mat,buf,sizeof(buf));
    
    ASSERT_TRUE(dec_mat.num_cols == 2);
    ASSERT_TRUE(dec_mat.num_rows == 3);
    ASSERT_TRUE(dec_mat.scalar_type == SimpleMatrixDataType_FLOAT64);

    ASSERT_TRUE(strcmp(dec_mat.device_id,"device_id") == 0);
    ASSERT_TRUE(strcmp(dec_mat.id,"name") == 0);
    
    ASSERT_NEAR(f[0],1.0,1e-6);
    ASSERT_NEAR(f[1],2.0,1e-6);
    ASSERT_NEAR(f[2],3.0,1e-6);
    ASSERT_NEAR(f[3],4.0,1e-6);
    ASSERT_NEAR(f[4],5.0,1e-6);
    ASSERT_NEAR(f[5],6.0,1e-6);


}

TEST_F(TestNetwork,TestEncodeDecodeFeatVec) {
    RadarMessage_t message;
    message.id = "foo-id";
    message.device_id = "device-id";
    message.sequence_number = 42;

    message.vec.push_back(3.14159);
    message.vec.push_back(1.0);
    message.vec.push_back(2.0);
    message.vec.push_back(3.0);
    message.vec.push_back(4.0);
    
    size_t message_size = 0;
    bytes = RadarMessageProtobuf::serialize_protobuf(message,message_size);
    
    ASSERT_TRUE(bytes);
    

    
    RadarMessage_t m;
    ASSERT_TRUE(RadarMessageProtobuf::deserialize_protobuf(bytes, message_size,m));
    
    ASSERT_EQ(m.vec.size(),message.vec.size());
    
    for (int i = 0; i < m.vec.size(); i++) {
        ASSERT_EQ(m.vec[i],message.vec[i]);
    }
    
    ASSERT_EQ(m.id, message.id);
    ASSERT_EQ(m.device_id, message.device_id);
    ASSERT_EQ(m.sequence_number, message.sequence_number);

}


