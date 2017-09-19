/*
 * 
 */

#include <algorithm>

#include "eviction/ARC.h"
#include "Cache.h"

ARC::ARC():
    cache_(nullptr)
{
}

void
ARC::setup(Cache* cache)
{
    cache_ = cache;
    c = cache_->getNrBlocks();
    p = 0;
}

void
ARC::state(std::stringstream& ss)
{
    ss << "|T1|=" << T1.size() << " |B1|=" << B1.size() << " |T2|=" << T2.size() << " |B2|=" << B2.size();
}

void
ARC::check()
{
    const int64_t T = T1.size() + T2.size();
    const int64_t L = T + B1.size() + B2.size();

    // Invariant: cannot have more items than the cache size
    if (T > c) {
        logging::error([this](std::stringstream& ss) { ss << "T > c, "; state(ss); });
    }
    // Invariant: balance point cannot be greater than the cache size
    if (p > c) {
        logging::error([this](std::stringstream& ss) { ss << "p > c, "; state(ss); });
    }
    // Invariant: all items cannot be greater than two times the cache size
    if (L > c * 2) {
        logging::error([this](std::stringstream& ss) { ss << "L > 2c, "; state(ss); });
    }
}

void
ARC::cacheHit(const Access* access, CacheEntry* cacheEntry)
{
    if (T1.has(cacheEntry)) {
        logging::debug([this](std::stringstream& ss) { ss << "hit1 "; state(ss); });
        T1.remove(cacheEntry);
        T2.put(cacheEntry);
    } else if (T2.has(cacheEntry)) {
        logging::debug([this](std::stringstream& ss) { ss << "hitM "; state(ss); });
        T2.remove(cacheEntry);
        T2.put(cacheEntry);
    } else {
        logging::error([this](std::stringstream& ss) { ss << "hit should have been in cache"; });
    }
}

void
ARC::cacheMiss(const Access* access, int64_t physicalBlock)
{
    if (B1.has(physicalBlock)) {
        logging::debug([this](std::stringstream& ss) { ss << "miss seen1 "; state(ss); });
        p = std::min(c, p + std::max(B2.size() / B1.size(), 1L));
        replace(physicalBlock);
        B1.remove(physicalBlock);
        CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
        T2.put(cacheEntry);

        check();

        return;
    }

    if (B2.has(physicalBlock)) {
        logging::debug([this](std::stringstream& ss) { ss << "miss seenM "; state(ss); });
        p = std::max(0L, p - std::max(B1.size() / B2.size(), 1L));
        replace(physicalBlock);
        B2.remove(physicalBlock);
        CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
        T2.put(cacheEntry);

        check();

        return;
    }

    logging::debug([this](std::stringstream& ss) { ss << "miss "; state(ss); });

    const int64_t L1 = T1.size() + B1.size();
    const int64_t L2 = T2.size() + B2.size();
    if (L1 >= c) {
        if (T1.size() < c) {
            B1.evictLeastRecent();
            replace(physicalBlock);
        } else {
            CacheEntry* entry = T1.evictLeastRecent();
            cache_->evictCacheEntry(entry);
        }
    } else if (L1+L2 >= c) {
        if (L1 + L2 == 2*c) {
            B2.evictLeastRecent();
        }
        replace(physicalBlock);
    }

    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
    T1.put(cacheEntry);

    check();
}

void
ARC::replace(int64_t physicalBlock)
{
    if (T1.size() >= 1
        && ((B2.has(physicalBlock) && T1.size() == p)
            || T1.size() > p))
    {
        CacheEntry* entry = T1.evictLeastRecent();
        B1.put(entry->physicalBlock);
        cache_->evictCacheEntry(entry);
        return;
    } else {
        CacheEntry* entry = T2.evictLeastRecent();
        B2.put(entry->physicalBlock);
        cache_->evictCacheEntry(entry);
    }
}

void
ARC::evict()
{
    replace(-1);
}
