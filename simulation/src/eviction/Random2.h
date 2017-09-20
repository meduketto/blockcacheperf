/*
 *
 */

#ifndef RANDOM2_H
#define RANDOM2_H 1

#include <vector>
#include <random>

#include "common.h"
#include "EvictionAlgorithm.h"

class Random2: public EvictionAlgorithm {
public:
    Random2();
    virtual void setup(Cache* cache);
    virtual void cacheHit(const Access* access, CacheEntry* cacheEntry);
    virtual void cacheMiss(const Access* access, int64_t physicalBlock);
    virtual void evict();

private:
    Cache* cache_;
    std::default_random_engine randomEngine_;
    std::vector<CacheEntry*> entries_;
};


#endif /* RANDOM2_H */
