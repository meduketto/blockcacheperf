/*
 * 
 */

#include <algorithm>

#include "eviction/2Q.h"
#include "Cache.h"

// FIXME: use clock rather than LRU timing

TwoQ::TwoQ():
    cache_(nullptr)
{
}

void
TwoQ::setup(Cache* cache)
{
    cache_ = cache;
    Kin = cache_->getNrBlocks() * DEFAULT_KIN;
    Kout = cache_->getNrBlocks() * DEFAULT_KOUT;
    logging::debug([this](std::stringstream& ss) { ss << "2Q params Kin=" << Kin << ", Kout=" << Kout; });
}

void
TwoQ::cacheHit(const Access* access, CacheEntry* cacheEntry)
{
    if (Am.has(cacheEntry)) {
        Am.put(cacheEntry);
    }
}

void
TwoQ::cacheMiss(const Access* access, int64_t physicalBlock)
{
    const bool seen = A1out.has(physicalBlock);

    if (cache_->isFull()) {
        reclaimFor(physicalBlock);
    }
    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
    if (seen) {
        Am.put(cacheEntry);
    } else {
        A1in.put(cacheEntry);
    }

    logging::debug([this](std::stringstream& ss) { ss << "|A1in|=" << A1in.size() <<
        ", |A1out|=" << A1out.size() << ", |Am|=" << Am.size(); });
}

void
TwoQ::reclaimFor(int64_t physicalBlock)
{
    if (A1in.size() > Kin) {
        CacheEntry* entry = A1in.evictLeastRecent();
        A1out.put(entry->physicalBlock);
        cache_->evictCacheEntry(entry);
        if (A1out.size() > Kout) {
            A1out.evictLeastRecent();
        }
    } else {
        CacheEntry* entry = Am.evictLeastRecent();
        cache_->evictCacheEntry(entry);
    }
}

void
TwoQ::evict()
{
    reclaimFor(-1);
}
