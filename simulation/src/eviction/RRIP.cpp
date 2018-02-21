/*
 * 
 */

#include "eviction/RRIP.h"
#include "Cache.h"

RRIP::RRIP(bool bimodal):
    cache_(nullptr),
    bimodal_(bimodal)
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
    // RRIP-HP: Hit Policy: near-immediate re-reference is predicted
    // This policy is found to be performing better in the paper
    cacheEntry->evictionValue = 0;
    // RRIP-FP: Frequency Policy: shorter re-reference is predicted
    // if (cacheEntry->evictionValue > 0) --cacheEntry->evictionValue;
}

int64_t
RRIP::predictReReference()
{
    if (bimodal_) {
        std::uniform_int_distribution<int> dist(0, 99);
        int dice = dist(randomEngine_);
        if (dice >= 95) {
            // Long Re-Reference is predicted for small number (~ %5)
            return 2;
        } else {
            // Distant Re-Reference is predicted for majority
            return 3;
        }
    } else {
        // Long Re-Reference is predicted
        return 2;
    }
}

void
RRIP::cacheMiss(const Access* access, int64_t physicalBlock)
{
    if (!cache_->isFull()) {
        CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
        cacheEntry->evictionValue = predictReReference();
        entries_.push_back(cacheEntry);
        return;
    }

    std::list<CacheEntry*>::iterator iter = findCandidate();
    CacheEntry* oldCacheEntry = *iter;
    cache_->evictCacheEntry(oldCacheEntry);

    CacheEntry* cacheNewEntry = cache_->loadCacheEntry(access, physicalBlock);
    cacheNewEntry->evictionValue = predictReReference();
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
