/*
 *
 */

#ifndef RRIP_H
#define RRIP_H 1

#include <list>

#include "common.h"
#include "EvictionAlgorithm.h"

class RRIP: public EvictionAlgorithm {
public:
    RRIP(bool dynamic);
    virtual void setup(Cache* cache);
    virtual void cacheHit(const Access* access, CacheEntry* cacheEntry);
    virtual void cacheMiss(const Access* access, int64_t physicalBlock);
    virtual void evict();

private:
    Cache* cache_;
    bool dynamic_;
    std::list<CacheEntry*> entries_;

    std::list<CacheEntry*>::iterator findCandidate();
};


#endif /* RRIP_H */
