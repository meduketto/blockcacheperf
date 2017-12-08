/*
 *
 */

#ifndef SPATIALCLOCK_H
#define SPATIALCLOCK_H 1

#include <vector>

#include "common.h"
#include "EvictionAlgorithm.h"

class SpatialClock: public EvictionAlgorithm {
public:
    SpatialClock();
    virtual void setup(Cache* cache);
    virtual void cacheHit(const Access* access, CacheEntry* cacheEntry);
    virtual void cacheMiss(const Access* access, int64_t physicalBlock);
    virtual void evict();

private:
    Cache* cache_;
    std::vector<CacheEntry*> entries_;
    unsigned int current_;
};


#endif /* SPATIALCLOCK_H */
