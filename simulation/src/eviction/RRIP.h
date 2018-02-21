/*
 *
 */

#ifndef RRIP_H
#define RRIP_H 1

#include <list>
#include <random>

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
    bool bimodal_;
    std::default_random_engine randomEngine_;
    std::list<CacheEntry*> entries_;

    int64_t predictReReference();
    std::list<CacheEntry*>::iterator findCandidate();
};


#endif /* RRIP_H */
