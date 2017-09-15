/*
 *
 */

#ifndef EVICTION_ALGORITHM_H
#define EVICTION_ALGORITHM_H 1

#include "common.h"

class Access;
class Cache;
class CacheEntry;

class EvictionAlgorithm {
public:
    virtual void setup(Cache* cache) = 0;
    virtual void cacheHit(const Access* access, CacheEntry* cacheEntry) = 0;
    virtual void cacheMiss(const Access* access, int64_t physicalBlock) = 0;
    virtual void evict() = 0;
};


#endif /* EVICTION_ALGORITHM_H */
