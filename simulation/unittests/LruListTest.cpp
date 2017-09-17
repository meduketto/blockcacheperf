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
    LruList<int64_t> test;
    ASSERT_EQ(test.size(), 0);
}

TEST_F(LruListTest, InOut)
{
    LruList<int64_t> test;
    test.put(1);
    test.put(2);
    test.put(3);
    test.put(4);
    test.put(5);
    ASSERT_EQ(5, test.size());
    ASSERT_TRUE(test.has(3));
    ASSERT_FALSE(test.has(6));
    ASSERT_EQ(1, test.evictLeastRecent());
    ASSERT_EQ(2, test.evictLeastRecent());
    ASSERT_EQ(3, test.evictLeastRecent());
    ASSERT_FALSE(test.has(3));
    ASSERT_EQ(4, test.evictLeastRecent());
    ASSERT_EQ(5, test.evictLeastRecent());
    ASSERT_EQ(0, test.size());
}

TEST_F(LruListTest, Hit)
{
    LruList<int64_t> test;
    test.put(1);
    test.put(2);
    test.put(3);

    test.put(1);
    ASSERT_EQ(2, test.evictLeastRecent());
    ASSERT_EQ(3, test.evictLeastRecent());
    ASSERT_EQ(1, test.evictLeastRecent());
    ASSERT_EQ(0, test.size());
}

TEST_F(LruListTest, Remove)
{
    LruList<int64_t> test;
    test.put(1);
    test.put(2);
    test.put(3);

    test.remove(2);
    ASSERT_EQ(1, test.evictLeastRecent());
    ASSERT_EQ(3, test.evictLeastRecent());
    ASSERT_EQ(0, test.size());
}

TEST_F(LruListTest, Combined)
{
    LruList<int64_t> test;
    test.put(1);
    test.put(2);
    test.put(1);
    test.put(3);
    test.put(4);
    test.put(2);
    test.put(5);
    test.put(5);
    test.put(3);
    ASSERT_EQ(5, test.size());

    ASSERT_EQ(1, test.evictLeastRecent());
    ASSERT_EQ(4, test.evictLeastRecent());
    ASSERT_EQ(2, test.evictLeastRecent());
    ASSERT_EQ(5, test.evictLeastRecent());
    ASSERT_EQ(3, test.evictLeastRecent());
    ASSERT_EQ(0, test.size());
}
