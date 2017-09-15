/*
 *
 */

#ifndef CACHE_H
#define CACHE_H 1

#include <vector>

#include "common.h"
#include "Access.h"

class EvictionAlgorithm;

class CacheEntry {
public:
    CacheEntry():
        physicalBlock(-1),
        evictionData(nullptr)
    {
    }

    int64_t physicalBlock;
    int64_t cachedTimeTick;
    int64_t lastAccessTimeTick;
    void* evictionData;
    bool isDirty;
};

class Cache {
public:
    Cache(int64_t blockSize, int64_t nrBlocks, EvictionAlgorithm* evictionAlgorithm);
    void handleAccess(const Access& access);
    void printStatistics();

    void evictCacheEntry(CacheEntry* cacheEntry);
    CacheEntry* loadCacheEntry(const Access* access, int64_t physicalBlock);
    int64_t getNrBlocks() const { return nrBlocks_; }
    int64_t getNrUsedBlocks() const { return nrUsedBlocks_; }

private:
    CacheEntry* findCacheEntry(int64_t physicalBlock);
    CacheEntry* newCacheEntry(int64_t physicalBlock);

    int64_t blockSize_;
    int64_t nrBlocks_;
    int64_t nrUsedBlocks_;
    EvictionAlgorithm* evictionAlgorithm_;
    int64_t timeTick_;
    int64_t nrReadAccesses_;
    int64_t nrWriteAccesses_;
    int64_t nrBlockReads_;
    int64_t nrBlockWrites_;

    std::vector<CacheEntry> cacheEntries;
};


#endif /* CACHE_H */
