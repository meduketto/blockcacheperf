/*
 * 
 */

#include <algorithm>

#include "eviction/CAR.h"
#include "Cache.h"

CAR::CAR():
    cache_(nullptr)
{
    T1hand = T1.end();
    T2hand = T2.end();
}

void
CAR::setup(Cache* cache)
{
    cache_ = cache;
    c = cache_->getNrBlocks();
    target = c / 2;
}

void
CAR::state(std::stringstream& ss)
{
    ss << "|T1|=" << T1.size() << " |B1|=" << B1.size() << " |T2|=" << T2.size() << " |B2|=" << B2.size() << " target=" << target;
}

void
CAR::cacheHit(const Access* access, CacheEntry* cacheEntry)
{
    // nothing to do
}

void
CAR::cacheMiss(const Access* access, int64_t physicalBlock)
{
    if (cache_->isFull()) {
        evict();
    }

    if (B1.has(physicalBlock)) {
        logging::debug([this](std::stringstream& ss) { ss << "miss B1 "; state(ss); });
        B1.remove(physicalBlock);
        CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
        cacheEntry->resetAccessBit();
        T2.push_back(cacheEntry);
        ++target;
        return;
    }

    if (B2.has(physicalBlock)) {
        logging::debug([this](std::stringstream& ss) { ss << "miss B2 "; state(ss); });
        B2.remove(physicalBlock);
        CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
        cacheEntry->resetAccessBit();
        T2.push_back(cacheEntry);
        --target;
        return;
    }

    logging::debug([this](std::stringstream& ss) { ss << "miss "; state(ss); });

    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
    cacheEntry->resetAccessBit();
    T1.push_back(cacheEntry);
}

bool
CAR::evictFromT1()
{
    while (T1.size() > 0) {
        if (T1hand == T1.end()) T1hand = T1.begin();

        std::list<CacheEntry*>::iterator entryHand(T1hand);
        CacheEntry* cacheEntry = *entryHand;
        ++T1hand;

        if (cacheEntry->isAccessed()) {
            cacheEntry->resetAccessBit();
            T1.erase(entryHand);
            T2.push_back(cacheEntry);
        } else {
            T1.erase(entryHand);
            B1.put(cacheEntry->physicalBlock);
            cache_->evictCacheEntry(cacheEntry);
            return true;
        }
    }
    return false;
}

bool
CAR::evictFromT2()
{
    while (T2.size() > 0) {
        if (T2hand == T2.end()) T2hand = T2.begin();

        std::list<CacheEntry*>::iterator entryHand(T2hand);
        CacheEntry* cacheEntry = *entryHand;
        ++T2hand;

        if (cacheEntry->isAccessed()) {
            cacheEntry->resetAccessBit();
        } else {
            T2.erase(entryHand);
            B2.put(cacheEntry->physicalBlock);
            cache_->evictCacheEntry(cacheEntry);
            return true;
        }
    }
    return false;
}

void
CAR::evict()
{
    if (T1.size() > target) {
        if (!evictFromT1()) evictFromT2();
    } else {
        if (!evictFromT2()) {
            if (!evictFromT1()) {
                evictFromT2();
            }
        }
    }

    if (T1.size() + B1.size() == c) {
        if (B1.size() > 0) B1.evictLeastRecent();
    } else {
        if (B2.size() > 0) B2.evictLeastRecent();
    }
}
