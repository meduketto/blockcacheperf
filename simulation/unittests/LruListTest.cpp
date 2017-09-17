/*
 * 
 */

#include <gtest/gtest.h>

#include "LruList.h"

class LruListTest: public ::testing::Test
{
public:
    LruListTest()
    {
    }
};

TEST_F(LruListTest, Empty)
{
    LruList<int64_t,int64_t> test;
}
