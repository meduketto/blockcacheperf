/*
 *
 */

#ifndef LRU_H
#define LRU_H 1

#include <list>

#include "common.h"
#include "EvictionAlgorithm.h"

class LRU: public EvictionAlgorithm {
public:
    LRU();
    virtual void setup(Cache* cache);
    virtual void cacheHit(const Access* access, CacheEntry* cacheEntry);
    virtual void cacheMiss(const Access* access, int64_t physicalBlock);
    virtual void evict();

private:
    Cache* cache_;
    std::list<CacheEntry*> entries_;
};


#endif /* LRU_H */
