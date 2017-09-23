/*
 *
 */

#ifndef TWO_Q_H
#define TWO_Q_H 1

#include "common.h"
#include "EvictionAlgorithm.h"
#include "LruList.h"

class TwoQ: public EvictionAlgorithm {
    // Following ratios are from (1994, T. Johnson, D. Shasha) paper
    // Ratio of recent cached blocks (A1in) to the cache size:
    static constexpr double DEFAULT_KIN = 0.25;
    // Ratio of remembered seen once blocks (A1out) to the cache size:
    static constexpr double DEFAULT_KOUT = 0.50;

public:
    TwoQ();
    virtual void setup(Cache* cache);
    virtual void cacheHit(const Access* access, CacheEntry* cacheEntry);
    virtual void cacheMiss(const Access* access, int64_t physicalBlock);
    virtual void evict();

private:
    void reclaimFor(int64_t physicalBlock);

    Cache* cache_;
    LruList<CacheEntry*> A1in; // Blocks accessed once recently
    LruList<int64_t> A1out;    // Blocks seen once but evicted
    LruList<CacheEntry*> Am;   // Blocks seen multiple times
    int64_t Kin;               // Max size of A1in
    int64_t Kout;              // Max size of A1out
};


#endif /* TWO_Q_H */
