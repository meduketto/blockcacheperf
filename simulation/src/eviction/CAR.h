/*
 *
 */

#ifndef CAR_H
#define CAR_H 1

#include <list>

#include "common.h"
#include "EvictionAlgorithm.h"
#include "LruList.h"

class CAR: public EvictionAlgorithm {
public:
    CAR();
    virtual void setup(Cache* cache);
    virtual void cacheHit(const Access* access, CacheEntry* cacheEntry);
    virtual void cacheMiss(const Access* access, int64_t physicalBlock);
    virtual void evict();

private:
    void state(std::stringstream& ss);
    void replace(int64_t physicalBlock);
    bool evictFromT1();
    bool evictFromT2();

    Cache* cache_;
    std::list<CacheEntry*> T1;
    std::list<CacheEntry*>::iterator T1hand;
    std::list<CacheEntry*> T2;
    std::list<CacheEntry*>::iterator T2hand;
    LruList<int64_t> B1;
    LruList<int64_t> B2;
    uint64_t c;
    uint64_t target;
};


#endif /* CAR_H */
