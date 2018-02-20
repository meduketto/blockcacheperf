/*
 * 
 */

#include "eviction/RRIP.h"
#include "Cache.h"

RRIP::RRIP(bool dynamic):
    cache_(nullptr),
    dynamic_(dynamic)
{
}

void
RRIP::setup(Cache* cache)
{
    cache_ = cache;
}

void
RRIP::cacheHit(const Access* access, CacheEntry* cacheEntry)
{
    cacheEntry->evictionValue = 0;
}

void
RRIP::cacheMiss(const Access* access, int64_t physicalBlock)
{
    if (!cache_->isFull()) {
        CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
        cacheEntry->evictionValue = 2;
        entries_.push_back(cacheEntry);
        return;
    }

    std::list<CacheEntry*>::iterator iter = findCandidate();
    CacheEntry* oldCacheEntry = *iter;
    cache_->evictCacheEntry(oldCacheEntry);

    CacheEntry* cacheNewEntry = cache_->loadCacheEntry(access, physicalBlock);
    cacheNewEntry->evictionValue = 2;
    *iter = cacheNewEntry;
}

std::list<CacheEntry*>::iterator
RRIP::findCandidate()
{
    while (1) {
        for (std::list<CacheEntry*>::iterator iter = entries_.begin(); iter != entries_.end(); ++iter) {
            CacheEntry* cacheEntry = *iter;
            if (cacheEntry->evictionValue == 3) return iter;
        }
        for (std::list<CacheEntry*>::iterator iter = entries_.begin(); iter != entries_.end(); ++iter) {
            CacheEntry* cacheEntry = *iter;
            ++cacheEntry->evictionValue;
        }
    }
}

void
RRIP::evict()
{
    if (entries_.size() > 0) {
        std::list<CacheEntry*>::iterator iter = findCandidate();
        CacheEntry* oldCacheEntry = *iter;
        cache_->evictCacheEntry(oldCacheEntry);
        entries_.erase(iter);
    }
}
