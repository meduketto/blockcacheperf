/*
 * 
 */

#include <algorithm>

#include "eviction/ClockPro.h"
#include "Cache.h"

class ClockProEntry {
public:
    ClockProEntry(CacheEntry* entry)
    {
        this->entry = entry;
        physicalBlock = entry->physicalBlock;
    }

    CacheEntry* entry;
    int64_t physicalBlock;
    bool isHot;
    bool isInTesting;
};

ClockPro::ClockPro():
    cache_(nullptr),
    nrCold_(0),
    nrHot_(0),
    nrTest_(0),
    nrNonResident_(0)
{
    handCold_ = entries_.end();
    handHot_ = entries_.end();
    handTest_ = entries_.end();
}

void
ClockPro::setup(Cache* cache)
{
    cache_ = cache;
    M_ = cache_->getNrBlocks();
    Mc_ = M_ / 2;
}

void
ClockPro::cacheHit(const Access* access, CacheEntry* cacheEntry)
{
    // nothing to do
}

void
ClockPro::insertListHead(ClockProEntry& entry)
{
    std::list<ClockProEntry>::iterator insertedIter;

    // Insert the entry into right position
    if (handHot_ == entries_.end()) {
        // No HANDhot, insert to actual list head
        entries_.push_back(entry);
        insertedIter = entries_.end();
        --insertedIter;
    } else {
        // There is a HANDhot, insert before it
        insertedIter = entries_.insert(handHot_, entry);
    }

    // Set the uninitialized hands and update counters
    if (entry.isHot) {
        if (handHot_ == entries_.end()) {
            handHot_ = insertedIter;
        }
        ++nrHot_;
    } else {
        if (handCold_ == entries_.end()) {
            handCold_ = insertedIter;
        }
        ++nrCold_;
    }
    if (entry.isInTesting) {
        if (handTest_ == entries_.end()) {
            handTest_ = insertedIter;
        }
        ++nrTest_;
    }
}

void
ClockPro::handHotNext()
{
    if (nrHot_ == 0) {
        handHot_ = entries_.end();
        return;
    }
    for (;;) {
        ++handHot_;
        if (handHot_ == entries_.end()) handHot_ = entries_.begin();
        if ((*handHot_).isHot) break;
    }
}

void
ClockPro::handColdNext()
{
    if (nrCold_ == 0) {
        handCold_ = entries_.end();
        return;
    }
    for (;;) {
        ++handCold_;
        if (handCold_ == entries_.end()) handCold_ = entries_.begin();
        const auto& entry = *handCold_;
        if ((!entry.isHot) && entry.entry) break;
    }
}

void
ClockPro::handTestNext()
{
    if (nrHot_ == 0) {
        handTest_ = entries_.end();
        return;
    }
    for (;;) {
        ++handTest_;
        if (handTest_ == entries_.end()) handTest_ = entries_.begin();
        if ((*handTest_).isInTesting) break;
    }
}

void
ClockPro::removeEntry(std::list<ClockProEntry>::iterator iter)
{
    const ClockProEntry& entry = *iter;

    // Update counters and hands
    if (entry.isHot) {
        --nrHot_;
        if (handHot_ == iter) {
            handHotNext();
        }
    } else {
        if (entry.entry) --nrCold_;
        if (handCold_ == iter) {
            handColdNext();
        }
    }
    if (entry.isInTesting) {
        --nrTest_;
        if (handTest_ == iter) {
            handTestNext();
        }
    }

    // Remove the entry
    entries_.erase(iter);
}

void
ClockPro::cacheMiss(const Access* access, int64_t physicalBlock)
{
    if (cache_->isFull()) {
        evict();
    }
    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);

    auto iter = seenBlocks_.find(physicalBlock);
    if (iter == seenBlocks_.end()) {
        // Not seen before -> COLD
        ClockProEntry entry(cacheEntry);
        entry.isHot = false;
        entry.isInTesting = true;
        insertListHead(entry);

        if (nrCold_ + nrTest_ >= M_ + Mc_) {
            moveHandTest();
        }
    } else {
        // Seen before -> HOT
        ClockProEntry entry(cacheEntry);
        entry.isHot = true;
        entry.isInTesting = false;
        removeEntry(iter->second);
        seenBlocks_.erase(iter);
        insertListHead(entry);

        moveHandHot();
    }
}

/*
 * Access Testing
 *    0      0       evict data - remove entry - stop at next cold page
 *    0      1       evict data - leave in list
 *    1      0       do nothing - reset abit move to head
 *    1      1       turn into hot - call hothand - reset abit move to head
 */
void
ClockPro::moveHandCold()
{
    if (handCold_ == entries_.end()) {
        logging::error([this](std::stringstream& ss) { ss << "no cold entry"; });
    }

    auto& entry = *handCold_;
    if (entry.isHot) {
        logging::error([this](std::stringstream& ss) { ss << "cold hand on hot entry"; });
    }

    if (entry.entry) {
        if (entry.entry->isAccessed()) {
            entry.entry->resetAccessBit();
            if (entry.isInTesting) {
                entry.isHot = true;
                entry.isInTesting = false;
                removeEntry(handCold_);
                insertListHead(entry);
                moveHandHot();
            } else {
                removeEntry(handCold_);
                insertListHead(entry);
            }
        } else {
            cache_->evictCacheEntry(entry.entry);
            entry.entry = nullptr;
            --nrCold_;
            if (entry.isInTesting) {
                ++nrNonResident_;
                seenBlocks_.insert(std::make_pair(entry.physicalBlock, handCold_));
            } else {
                auto iter = handCold_;
                ++handCold_;
                removeEntry(iter);
            }
        }
    }

    handColdNext();
}

/*
 * a=0  turn into cold
 * a=1 reset abit
 * cold page -> terminate test period
 * cold-no data -> remove entry
 */
void
ClockPro::moveHandHot()
{
    while (true) {
        if (handHot_ == entries_.end()) break;

        auto& entry = *handHot_;
        if (entry.isHot) {
            if (entry.entry->isAccessed()) {
                entry.entry->resetAccessBit();
            } else {
                entry.isHot = false;
                handHotNext();
                if (nrNonResident_ >= M_) {
                    moveHandTest();
                }
                return;
            }
        } else {
            entry.isInTesting = false;
            if (!entry.entry) {
                --nrNonResident_;
                seenBlocks_.erase(seenBlocks_.find(entry.physicalBlock));
                removeEntry(handHot_);
                continue;
            }
        }
        handHotNext();
    }
}

/*
 * cold page -> terminate test period
 * cold-no data -> remove entry
 */
void
ClockPro::moveHandTest()
{
    if (handTest_ == entries_.end()) return;

    auto& entry = *handTest_;
    entry.isInTesting = false;
    if (!entry.entry) {
        --nrNonResident_;
        seenBlocks_.erase(seenBlocks_.find(entry.physicalBlock));
        removeEntry(handHot_);
    }

    handTestNext();
}

void
ClockPro::evict()
{
    while (cache_->isFull()) {
        moveHandCold();
    }
}
