/*
 * 
 */

#include <algorithm>

#include "eviction/LRU.h"
#include "Cache.h"

LRU::LRU():
    cache_(nullptr)
{
}

void
LRU::setup(Cache* cache)
{
    cache_ = cache;
}

void
LRU::cacheHit(const Access* access, CacheEntry* cacheEntry)
{
    entries_.put(cacheEntry);
}

void
LRU::cacheMiss(const Access* access, int64_t physicalBlock)
{
    if (cache_->isFull()) {
        evict();
    }
    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
    entries_.put(cacheEntry);
}

void
LRU::evict()
{
    CacheEntry* entry = entries_.evictLeastRecent();
    cache_->evictCacheEntry(entry);
}
