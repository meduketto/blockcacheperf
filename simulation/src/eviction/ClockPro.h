/*
 *
 */

#ifndef CLOCKPRO_H
#define CLOCKPRO_H 1

#include <list>
#include <unordered_map>

#include "common.h"
#include "EvictionAlgorithm.h"

class ClockProEntry;

class ClockPro: public EvictionAlgorithm {
public:
    ClockPro();
    virtual void setup(Cache* cache);
    virtual void cacheHit(const Access* access, CacheEntry* cacheEntry);
    virtual void cacheMiss(const Access* access, int64_t physicalBlock);
    virtual void evict();

private:
    void insertListHead(ClockProEntry& entry);
    void removeEntry(std::list<ClockProEntry>::iterator iter);
    void handHotNext();
    void handColdNext();
    void handTestNext();
    void moveHandCold();
    void moveHandHot();
    void moveHandTest();

    Cache* cache_;
    std::list<ClockProEntry> entries_;
    std::list<ClockProEntry>::iterator handCold_;
    std::list<ClockProEntry>::iterator handHot_;
    std::list<ClockProEntry>::iterator handTest_;
    std::unordered_map<int64_t,typename std::list<ClockProEntry>::iterator> seenBlocks_;
    int nrCold_;
    int nrHot_;
    int nrTest_;
    int nrNonResident_;
    int M_;
    int Mc_;
};


#endif /* CLOCKPRO_H */
