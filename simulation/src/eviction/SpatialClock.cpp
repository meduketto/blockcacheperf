/*
 * 
 */

#include <algorithm>

#include "eviction/SpatialClock.h"
#include "Cache.h"

SpatialClock::SpatialClock():
    cache_(nullptr),
    current_(0)
{
}

void
SpatialClock::setup(Cache* cache)
{
    cache_ = cache;
}

void
SpatialClock::cacheHit(const Access* access, CacheEntry* cacheEntry)
{
    // nothing to do
}

void
SpatialClock::cacheMiss(const Access* access, int64_t physicalBlock)
{
    if (cache_->isFull()) {
        evict();
    }
    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
    entries_.insert(
        std::upper_bound(entries_.begin(), entries_.end(), cacheEntry,
            [](const CacheEntry* a, const CacheEntry* b) { return a->physicalBlock < b->physicalBlock; }
        ),
        cacheEntry
    );
}

void
SpatialClock::evict()
{
    while (true) {
        if (current_ >= entries_.size()) current_ = 0;
        if (current_ >= entries_.size()) return;

        unsigned int victim = current_++;
        CacheEntry* entry = entries_[victim];

        if (entry->isAccessed()) {
            entry->resetAccessBit();
        } else {
            entries_.erase(entries_.begin() + victim);
            cache_->evictCacheEntry(entry);
            return;
        }
    }
}
