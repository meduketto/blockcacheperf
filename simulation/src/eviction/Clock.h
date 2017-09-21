/*
 *
 */

#ifndef CLOCK_H
#define CLOCK_H 1

#include <list>

#include "common.h"
#include "EvictionAlgorithm.h"

class Clock: public EvictionAlgorithm {
public:
    Clock();
    virtual void setup(Cache* cache);
    virtual void cacheHit(const Access* access, CacheEntry* cacheEntry);
    virtual void cacheMiss(const Access* access, int64_t physicalBlock);
    virtual void evict();

private:
    Cache* cache_;
    std::list<CacheEntry*> entries_;
    std::list<CacheEntry*>::iterator hand_;
};


#endif /* CLOCK2_H */
