#include "gtest/gtest.h"
#include "circbuf.h"

class TestBuffers : public ::testing::Test {
protected:
    
    
    virtual void SetUp() {
    }
    
    virtual void TearDown() {
    }
    
};

class DISABLED_TestBuffers : public TestBuffers {};


TEST_F(TestBuffers, TestCircularBuffer) {
    circular_buffer<int> buf(3);
    
    int i = 1;
    
    ASSERT_FALSE(buf.is_full());
    buf.push_back(i++);
    ASSERT_FALSE(buf.is_full());
    buf.push_back(i++);
    ASSERT_FALSE(buf.is_full());
    buf.push_back(i++);
    ASSERT_TRUE(buf.is_full());
    
    ASSERT_TRUE(buf[0] == 1);
    ASSERT_TRUE(buf[1] == 2);
    ASSERT_TRUE(buf[2] == 3);

       
    buf.push_back(i++);
    ASSERT_TRUE(buf.is_full());
    ASSERT_TRUE(buf[0] == 2);
    ASSERT_TRUE(buf[1] == 3);
    ASSERT_TRUE(buf[2] == 4);

    ASSERT_TRUE(buf[-1] == 4);
    ASSERT_TRUE(buf[-2] == 3);
    ASSERT_TRUE(buf[-3] == 2);

}

