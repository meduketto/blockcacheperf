/*
 *
 */

#ifndef CACHE_H
#define CACHE_H 1

#include <vector>
#include <unordered_map>

#include "common.h"
#include "Access.h"

class EvictionAlgorithm;

class CacheEntry {
    friend class Cache;

public:
    CacheEntry():
        physicalBlock(-1),
        evictionData(nullptr),
        nextEmpty_(nullptr)
    {
    }

    int64_t physicalBlock;
    int64_t cachedTimeTick;
    int64_t lastAccessTimeTick;
    void* evictionData;
    bool isDirty;

private:
    CacheEntry* nextEmpty_;
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
    bool isFull() const { return nrUsedBlocks_ >= nrBlocks_; }

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

    CacheEntry* nextEmpty_;
    std::vector<CacheEntry> cacheEntries_;
    std::unordered_map<int64_t,CacheEntry*> cacheMap_;
};


#endif /* CACHE_H */
