/*
 *
 */

#ifndef FAB_H
#define FAB_H 1

#include <vector>

#include "common.h"
#include "EvictionAlgorithm.h"
#include "LruList.h"

class FlashBlock;

class FAB: public EvictionAlgorithm {
public:
    FAB();
    virtual void setup(Cache* cache);
    virtual void cacheHit(const Access* access, CacheEntry* cacheEntry);
    virtual void cacheMiss(const Access* access, int64_t physicalBlock);
    virtual void evict();

private:
    int64_t getPhysicalByte(int64_t physicalBlock);

    Cache* cache_;
    std::list<FlashBlock*> blocks_;
    std::unordered_map<int64_t,typename std::list<FlashBlock*>::iterator> blockMap_;
};


#endif /* FAB_H */
