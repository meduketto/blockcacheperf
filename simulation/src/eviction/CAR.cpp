/*
 * 
 */

#include <algorithm>

#include "eviction/CAR.h"
#include "Cache.h"

CAR::CAR():
    cache_(nullptr),
    c(0),
    p(0)
{
}

void
CAR::setup(Cache* cache)
{
    cache_ = cache;
    c = cache_->getNrBlocks();
}

void
CAR::state(std::stringstream& ss)
{
    ss << "|T1|=" << T1.size() << " |B1|=" << B1.size() << " |T2|=" << T2.size() << " |B2|=" << B2.size() << " p=" << p;
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
        if (!B1.has(physicalBlock) && !B2.has(physicalBlock)) {
            evictDirectory();
        }
    }

    if (B1.has(physicalBlock)) {
        logging::debug([this](std::stringstream& ss) { ss << "miss B1 "; state(ss); });
        p = std::min(p + std::max(1L, B2.size() / B1.size()), (int64_t)c);
        B1.remove(physicalBlock);
        CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
        cacheEntry->resetAccessBit();
        T2.push_back(cacheEntry);
        return;
    }

    if (B2.has(physicalBlock)) {
        logging::debug([this](std::stringstream& ss) { ss << "miss B2 "; state(ss); });
        p = std::max(p - std::max(1L, B1.size() / B2.size()), 0L);
        B2.remove(physicalBlock);
        CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
        cacheEntry->resetAccessBit();
        T2.push_back(cacheEntry);
        return;
    }

    logging::debug([this](std::stringstream& ss) { ss << "miss "; state(ss); });

    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
    cacheEntry->resetAccessBit();
    T1.push_back(cacheEntry);
}

void
CAR::evictDirectory()
{
    if (T1.size() + B1.size() == c) {
        if (B1.size() > 0) B1.evictLeastRecent();
    } else {
        if (B2.size() > 0) B2.evictLeastRecent();
    }
}

void
CAR::evict()
{
    while (T1.size() + T2.size() > 0) {
        if (T1.size() >= (uint64_t) std::max(1L, p)) {
            CacheEntry* cacheEntry = T1.front();
            T1.pop_front();
            if (cacheEntry->isAccessed()) {
                cacheEntry->resetAccessBit();
                T2.push_back(cacheEntry);
            } else {
                B1.put(cacheEntry->physicalBlock);
                cache_->evictCacheEntry(cacheEntry);
                return;
            }
        } else {
            CacheEntry* cacheEntry = T2.front();
            T2.pop_front();
            if (cacheEntry->isAccessed()) {
                cacheEntry->resetAccessBit();
                T2.push_back(cacheEntry);
            } else {
                B2.put(cacheEntry->physicalBlock);
                cache_->evictCacheEntry(cacheEntry);
                return;
            }
        }
    }
}
