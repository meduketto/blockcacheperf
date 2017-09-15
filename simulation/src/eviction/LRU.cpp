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
    std::list<CacheEntry*>::iterator iter = std::find(entries_.begin(), entries_.end(), cacheEntry);
    entries_.erase(iter);
    entries_.push_back(cacheEntry);
}

void
LRU::cacheMiss(const Access* access, int64_t physicalBlock)
{
    if (cache_->getNrUsedBlocks() >= cache_->getNrBlocks()) {
        evict();
    }
    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
    entries_.push_back(cacheEntry);
}

void
LRU::evict()
{
    CacheEntry* entry = entries_.front();
    entries_.pop_front();
    cache_->evictCacheEntry(entry);
}
