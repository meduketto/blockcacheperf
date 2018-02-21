/*
 * 
 */

#include <algorithm>

#include "eviction/FAB.h"
#include "Cache.h"

class FlashBlock {
public:
    // These must be 2^n
    static constexpr int64_t blockSize = 16 * 4096;
    static constexpr int64_t pageSize = 512;
    static constexpr int nrPagesPerBlock = blockSize / pageSize;

    FlashBlock(int64_t blockNo);

    int64_t blockNo_;
    int nrPages_;
    CacheEntry* pages_[nrPagesPerBlock];
};

FlashBlock::FlashBlock(int64_t blockNo):
    blockNo_(blockNo),
    nrPages_(0)
{
    for(int i = 0; i < nrPagesPerBlock; ++i) {
        pages_[i] = nullptr;
    }
}

//

FAB::FAB():
    cache_(nullptr)
{
}

int64_t
FAB::getPhysicalByte(int64_t physicalBlock)
{
    return (physicalBlock * cache_->getBlockSize()) >> 9;
}

void
FAB::setup(Cache* cache)
{
    cache_ = cache;
}

void
FAB::cacheHit(const Access* access, CacheEntry* cacheEntry)
{
    const int64_t blockNo = getPhysicalByte(cacheEntry->physicalBlock) / FlashBlock::blockSize;

    auto iter = blockMap_.find(blockNo);
    if (iter->second == blocks_.begin()) return;
    FlashBlock* flashBlock = *iter->second;
    blocks_.erase(iter->second);
    blockMap_.erase(iter);
    blocks_.push_front(flashBlock);
    blockMap_.insert(std::make_pair(blockNo, blocks_.begin()));
}

void
FAB::cacheMiss(const Access* access, int64_t physicalBlock)
{
    if (cache_->isFull()) {
        evict();
    }

    const int64_t blockNo = getPhysicalByte(physicalBlock) / FlashBlock::blockSize;
    const int64_t pageNo = (getPhysicalByte(physicalBlock) % FlashBlock::blockSize) / FlashBlock::pageSize;
    auto iter = blockMap_.find(blockNo);
    FlashBlock* flashBlock = nullptr;
    if (iter == blockMap_.end()) {
        flashBlock = new FlashBlock(blockNo);
        blocks_.push_front(flashBlock);
        blockMap_.insert(std::make_pair(blockNo, blocks_.begin()));
    } else {
        flashBlock = *iter->second;
    }

    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);

    ++flashBlock->nrPages_;
    flashBlock->pages_[pageNo] = cacheEntry;
}

void
FAB::evict()
{
    int nrPages = -1;
    FlashBlock* victim = nullptr;

    for (auto iter = blocks_.rbegin(); iter != blocks_.rend(); ++iter) {
        FlashBlock* flashBlock = *iter;
        if (flashBlock->nrPages_ == FlashBlock::nrPagesPerBlock) {
            victim = flashBlock;
            break;
        }
        if (flashBlock->nrPages_ > nrPages) {
            victim = flashBlock;
            nrPages = victim->nrPages_;
        }
    }

    for (int i = 0; i < FlashBlock::nrPagesPerBlock; ++i) {
        CacheEntry* cacheEntry = victim->pages_[i];
        if (cacheEntry) cache_->evictCacheEntry(cacheEntry);
    }

    auto iter = blockMap_.find(victim->blockNo_);
    blocks_.erase(iter->second);
    blockMap_.erase(iter);
}
