/*
 * 
 */

#include <gtest/gtest.h>

#include "Cache.h"
#include "EvictionAlgorithm.h"

class CacheTest: public ::testing::Test
{
public:
    CacheTest()
    {
    }
};

class TestEviction: public EvictionAlgorithm {
public:
    TestEviction() { }

    virtual void setup(Cache* cache) { }

    virtual void cacheHit(const Access* access, CacheEntry* cacheEntry) { }
    virtual void cacheMiss(const Access* access, int64_t physicalBlock) { }
    virtual void evict() { }
};

TEST_F(CacheTest, Empty)
{
    TestEviction evict;

    Cache cache(4096, 10, &evict);


}
