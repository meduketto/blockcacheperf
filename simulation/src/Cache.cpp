/*
 * 
 */

#include <stdio.h>

#include "Cache.h"
#include "EvictionAlgorithm.h"

Cache::Cache(int64_t blockSize, int64_t nrBlocks, EvictionAlgorithm* evictionAlgorithm):
    blockSize_(blockSize),
    nrBlocks_(nrBlocks),
    nrUsedBlocks_(0),
    evictionAlgorithm_(evictionAlgorithm),
    timeTick_(0),
    nrReadAccesses_(0),
    nrWriteAccesses_(0),
    nrBlockReads_(0),
    nrBlockWrites_(0),
    outputFile_(nullptr)
{
    for (int i = 0; i < nrBlocks; ++i) {
        cacheEntries_.emplace_back(CacheEntry());
    }
    nextEmpty_ = &cacheEntries_[0];
    for (int i = 0; i < nrBlocks - 1; ++i) {
        cacheEntries_[i].nextEmpty_ = &cacheEntries_[i+1];
    }

    evictionAlgorithm_->setup(this);
}

void
Cache::setOutputFile(const char* filename)
{
    outputFile_ = fopen(filename, "w");
    if (!outputFile_) {
        logging::error([](std::stringstream& ss) { ss << "cannot open output file"; });
        return;
    }
    fprintf(outputFile_, "# Post-cache traces\n");
}

void
Cache::handleAccess(const Access& access)
{
    // Every access increments the time
    ++timeTick_;

    if (access.isRead()) {
        ++nrReadAccesses_;
    } else {
        ++nrWriteAccesses_;
    }

    // Find the block being asked
    const int64_t physicalBlock = (access.sector << 9) / blockSize_;
    CacheEntry* cacheEntry = findCacheEntry(physicalBlock);

    if (cacheEntry) {
        // Cache Hit :)
        cacheEntry->lastAccessTimeTick = timeTick_;
        if (access.isRead()) {
            cacheEntry->accessBit_ = true;
        } else {
            cacheEntry->isDirty = true;
        }
        evictionAlgorithm_->cacheHit(&access, cacheEntry);

    } else {
        // Cache Miss :(
        evictionAlgorithm_->cacheMiss(&access, physicalBlock);

    }
}

void
Cache::printStatistics()
{
    printf("# Read accesses = %ld\n"
           "# Write accesses = %ld\n"
           "# Blocks read = %ld\n"
           "# Blocks written = %ld\n",
           nrReadAccesses_,
           nrWriteAccesses_,
           nrBlockReads_,
           nrBlockWrites_);
}

CacheEntry*
Cache::findCacheEntry(int64_t physicalBlock)
{
    auto iter = cacheMap_.find(physicalBlock);
    if (iter != cacheMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

CacheEntry*
Cache::newCacheEntry(int64_t physicalBlock)
{
    if (!nextEmpty_) {
        logging::error([](std::stringstream& ss) { ss << "algorithm overloads the cache"; });
    }

    CacheEntry* cacheEntry = nextEmpty_;
    nextEmpty_ = cacheEntry->nextEmpty_;
    cacheEntry->nextEmpty_ = nullptr;

    ++nrUsedBlocks_;
    cacheEntry->physicalBlock = physicalBlock;
    cacheEntry->cachedTimeTick = timeTick_;
    cacheEntry->lastAccessTimeTick = timeTick_;
    cacheEntry->evictionPtr = nullptr;
    cacheEntry->evictionValue = -1;
    cacheEntry->isDirty = false;
    cacheEntry->accessBit_ = false;
    cacheMap_.insert(std::make_pair(physicalBlock, cacheEntry));
    return cacheEntry;
}

void
Cache::evictCacheEntry(CacheEntry* cacheEntry)
{
    if (cacheEntry->physicalBlock == -1) {
        logging::error([](std::stringstream& ss) { ss << "algorithm tries to evict bad cache block"; });
    }

    auto iter = cacheMap_.find(cacheEntry->physicalBlock);
    if (iter == cacheMap_.end()) {
        logging::error([](std::stringstream& ss) { ss << "algorithm tries to evict unknown cache block"; });
    }
    cacheMap_.erase(iter);

    if (cacheEntry->isDirty) {
        // There is new data on this block
        // In real world, it would have been flushed to disk.
        nrBlockWrites_++;
        if (outputFile_) fprintf(outputFile_, "w %ld\n", cacheEntry->physicalBlock);
    }

    cacheEntry->physicalBlock = -1;

    cacheEntry->nextEmpty_ = nextEmpty_;
    nextEmpty_ = cacheEntry;

    --nrUsedBlocks_;
}

CacheEntry*
Cache::loadCacheEntry(const Access* access, int64_t physicalBlock)
{
    CacheEntry* cacheEntry = newCacheEntry(physicalBlock);
    // A read cache miss loads the block from the disk
    // Writes are delayed until they are evicted
    if (access->isRead()) {
        if (outputFile_) fprintf(outputFile_, "r %ld\n", cacheEntry->physicalBlock);
        ++nrBlockReads_;
    } else {
        cacheEntry->isDirty = true;
    }
    return cacheEntry;
}
