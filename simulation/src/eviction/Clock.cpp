/*
 * 
 */

#include "eviction/Clock.h"
#include "Cache.h"

Clock::Clock():
    cache_(nullptr)
{
    hand_ = entries_.end();
}

void
Clock::setup(Cache* cache)
{
    cache_ = cache;
}

void
Clock::cacheHit(const Access* access, CacheEntry* cacheEntry)
{
    // nothing to do
}

void
Clock::cacheMiss(const Access* access, int64_t physicalBlock)
{
    if (cache_->isFull()) {
        evict();
    }
    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
    entries_.push_front(cacheEntry);
}

void
Clock::evict()
{
    if (entries_.size() == 0) {
        logging::error([](std::stringstream& ss) { ss << "Clock: shouldn't evict when cache is empty"; });
    }

    while (true) {
        if (hand_ == entries_.end()) hand_ = entries_.begin();

        std::list<CacheEntry*>::iterator entryHand(hand_);
        CacheEntry* entry = *entryHand;
        ++hand_;

        if (entry->isAccessed()) {
            entry->resetAccessBit();
        } else {
            entries_.erase(entryHand);
            cache_->evictCacheEntry(entry);
            return;
        }
    }
}
