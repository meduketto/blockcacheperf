/*
 * 
 */

#include <algorithm>

#include "eviction/FAB.h"
#include "Cache.h"

class FlashBlock {
public:
    // These must be 2^n
    static constexpr int64_t blockSize = 16 * 1024;
    static constexpr int64_t pageSize = 4096;
    static constexpr int nrPagesPerBlock = blockSize / pageSize;

    FlashBlock(int64_t blockNo);

    int64_t blockNo_;
    int nrPages_;
    int nrDirtyPages_;
    CacheEntry* pages_[nrPagesPerBlock];
    bool dirty_[nrPagesPerBlock];
};

FlashBlock::FlashBlock(int64_t blockNo):
    blockNo_(blockNo),
    nrPages_(0),
    nrDirtyPages_(0)
{
    for(int i = 0; i < nrPagesPerBlock; ++i) {
        pages_[i] = nullptr;
        dirty_[i] = false;
    }
}

//

FAB::FAB():
    cache_(nullptr)
{
}

struct FlashAddress
FAB::getFlashAddress(int64_t physicalBlock)
{
    struct FlashAddress address;
    int64_t byte = physicalBlock * cache_->getBlockSize();
    address.blockNo = byte / FlashBlock::blockSize;
    address.pageNo = (byte - (address.blockNo * FlashBlock::blockSize)) / FlashBlock::pageSize;
    return address;
}

void
FAB::setup(Cache* cache)
{
    cache_ = cache;
}

void
FAB::cacheHit(const Access* access, CacheEntry* cacheEntry)
{
    FlashAddress addr = getFlashAddress(cacheEntry->physicalBlock);

    logging::debug([&](std::stringstream& ss) { ss << "hit block:" << addr.blockNo << " page:" << addr.pageNo << " physical:" << cacheEntry->physicalBlock; });

    auto iter = blockMap_.find(addr.blockNo);
    FlashBlock* flashBlock = *iter->second;

    if (access->isWrite() && !flashBlock->dirty_[addr.pageNo]) {
        flashBlock->dirty_[addr.pageNo] = true;
        ++flashBlock->nrDirtyPages_;
    }

    blocks_.erase(iter->second);
    blockMap_.erase(iter);
    blocks_.push_front(flashBlock);
    blockMap_.insert(std::make_pair(addr.blockNo, blocks_.begin()));
}

void
FAB::cacheMiss(const Access* access, int64_t physicalBlock)
{
    if (cache_->isFull()) {
        evict();
    }

    FlashAddress addr = getFlashAddress(physicalBlock);

    logging::debug([&](std::stringstream& ss) { ss << "hit miss:" << addr.blockNo << " page:" << addr.pageNo << " physical:" << physicalBlock; });

    auto iter = blockMap_.find(addr.blockNo);
    FlashBlock* flashBlock = nullptr;
    if (iter == blockMap_.end()) {
        flashBlock = new FlashBlock(addr.blockNo);
        blocks_.push_front(flashBlock);
        blockMap_.insert(std::make_pair(addr.blockNo, blocks_.begin()));
    } else {
        flashBlock = *iter->second;
    }

    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);

    ++flashBlock->nrPages_;
    if (access->isWrite()) {
        flashBlock->dirty_[addr.pageNo] = true;
        ++flashBlock->nrDirtyPages_;
        logging::debug([&](std::stringstream& ss) { ss << "dirty:" << flashBlock->nrDirtyPages_ << " total:" << flashBlock->nrPages_; });
    }
    flashBlock->pages_[addr.pageNo] = cacheEntry;
}

void
FAB::evict()
{
    int score = -1;
    FlashBlock* victim = nullptr;

    for (auto iter = blocks_.rbegin(); iter != blocks_.rend(); ++iter) {
        FlashBlock* flashBlock = *iter;
        if (flashBlock->nrDirtyPages_ == FlashBlock::nrPagesPerBlock) {
            victim = flashBlock;
            break;
        }
        int fbScore = flashBlock->nrPages_;
        if (fbScore > score) {
            victim = flashBlock;
            score = fbScore;
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
