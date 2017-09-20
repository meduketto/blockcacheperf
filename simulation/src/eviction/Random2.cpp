/*
 * 
 */

#include <algorithm>

#include "eviction/Random2.h"
#include "Cache.h"

// FIXME: use clock rather than LRU timing

Random2::Random2():
    cache_(nullptr)
{
}

void
Random2::setup(Cache* cache)
{
    cache_ = cache;
}

void
Random2::cacheHit(const Access* access, CacheEntry* cacheEntry)
{
    // nothing to do
}

void
Random2::cacheMiss(const Access* access, int64_t physicalBlock)
{
    if (cache_->isFull()) {
        evict();
    }
    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
    entries_.push_back(cacheEntry);
}

void
Random2::evict()
{
    if (entries_.size() == 0) {
        logging::error([](std::stringstream& ss) { ss << "Random2: shouldn't evict when cache is empty"; });
    }

    std::uniform_int_distribution<int> dist(0, entries_.size() - 1);
    int entryPos = dist(randomEngine_);
    CacheEntry* entry = entries_[entryPos];
    while (true) {
        int entryPos2 = dist(randomEngine_);
        CacheEntry* entry2 = entries_[entryPos2];
        if (entry != entry2) {
            if (entry->lastAccessTimeTick < entry2->lastAccessTimeTick) {
                entry = entry2;
                entryPos = entryPos2;
            }
            break;
        }
    }

    entries_.erase(entries_.begin() + entryPos);

    cache_->evictCacheEntry(entry);
}
