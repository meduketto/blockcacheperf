/*
 *
 */

#ifndef ARC_H
#define ARC_H 1

#include <list>

#include "common.h"
#include "EvictionAlgorithm.h"
#include "LruList.h"

class ARC: public EvictionAlgorithm {
public:
    ARC();
    virtual void setup(Cache* cache);
    virtual void cacheHit(const Access* access, CacheEntry* cacheEntry);
    virtual void cacheMiss(const Access* access, int64_t physicalBlock);
    virtual void evict();

private:
    void check();
    void replace(int64_t physicalBlock);

    Cache* cache_;
    LruList<CacheEntry*,CacheEntry*> T1;
    LruList<CacheEntry*,CacheEntry*> T2;
    LruList<int64_t,int64_t> B1;
    LruList<int64_t,int64_t> B2;
    int64_t c;
    int64_t p;
};


#endif /* ARC_H */
