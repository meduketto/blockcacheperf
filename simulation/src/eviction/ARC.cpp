/*
 * 
 */

#include <algorithm>

#include "eviction/ARC.h"
#include "Cache.h"

static bool debug = false;

ARC::ARC():
    cache_(nullptr)
{
}

void
ARC::setup(Cache* cache)
{
    cache_ = cache;
    c = cache_->getNrBlocks();
    p = 0;
}

void
ARC::check()
{
    int64_t L = T1.size() + T2.size();
    int64_t F = L + B1.size() + B2.size();
    if (L > c) {
printf("T1 %lu B1 %lu T2 %lu B2 %lu p %ld\n", T1.size(), B1.size(), T2.size(), B2.size(), p);
        printf("Larger L %ld\n", L);
    exit(0);
    }
    if (p > c) {
printf("T1 %lu B1 %lu T2 %lu B2 %lu p %ld\n", T1.size(), B1.size(), T2.size(), B2.size(), p);
        printf("Larger p %ld\n", p);
    exit(0);
    }
    if (F > c * 2) {
printf("T1 %lu B1 %lu T2 %lu B2 %lu p %ld\n", T1.size(), B1.size(), T2.size(), B2.size(), p);
        printf("Larger F %ld\n", F);
    exit(0);
    }
}

void
ARC::cacheHit(const Access* access, CacheEntry* cacheEntry)
{
    check();

    const int64_t G1 = T1.size() + T2.size() + B1.size() + B2.size();

    if (T1.has(cacheEntry)) {
if (debug) printf("hit1 block %ld\n", cacheEntry->physicalBlock);
        T1.remove(cacheEntry);
        T2.put(cacheEntry);
    } else if (T2.has(cacheEntry)) {
if (debug) printf("hit2 block %ld\n", cacheEntry->physicalBlock);
        T2.remove(cacheEntry);
        T2.put(cacheEntry);
    } else {
        printf("Hit should have been in cache.\n");
    }
if (debug) printf("T1 %lu B1 %lu T2 %lu B2 %lu p %ld\n", T1.size(), B1.size(), T2.size(), B2.size(), p);

    const int64_t G2 = T1.size() + T2.size() +  B1.size() + B2.size();
    if (G1 != G2) { printf("break\n"); exit(0); }

}

void
ARC::cacheMiss(const Access* access, int64_t physicalBlock)
{
    check();

    const int64_t G1 = T1.size() + T2.size() + B1.size() + B2.size();

    if (B1.has(physicalBlock)) {
if (debug) printf("miss seen1 block %ld\n", physicalBlock);
        p = std::min(c, p + std::max(B2.size() / B1.size(), 1L));
        replace(physicalBlock);
        B1.remove(physicalBlock);
        CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
        T2.put(cacheEntry);
if (debug) printf("T1 %lu B1 %lu T2 %lu B2 %lu p %ld\n", T1.size(), B1.size(), T2.size(), B2.size(), p);
    const int64_t G2 = T1.size() + T2.size() + B1.size() + B2.size();
    if (G1 != G2) { printf("break\n"); exit(0); }

        return;
    }

    if (B2.has(physicalBlock)) {
if (debug) printf("miss seen2 block %ld\n", physicalBlock);
        p = std::max(0L, p - std::max(B1.size() / B2.size(), 1L));
        replace(physicalBlock);
        B2.remove(physicalBlock);
        CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
        T2.put(cacheEntry);
if (debug) printf("T1 %lu B1 %lu T2 %lu B2 %lu p %ld\n", T1.size(), B1.size(), T2.size(), B2.size(), p);
    const int64_t G2 = T1.size() + T2.size() + B1.size() + B2.size();
    if (G1 != G2) { printf("break\n"); exit(0); }

        return;
    }

if (debug) printf("miss unseen block %ld\n", physicalBlock);

    const int64_t L1 = T1.size() + B1.size();
    const int64_t L2 = T2.size() + B2.size();
    if (L1 >= c) {
        if (T1.size() < c) {
            B1.evictLeastRecent();
            replace(physicalBlock);
        } else {
            CacheEntry* entry = T1.evictLeastRecent();
//            B1.put(entry->physicalBlock, entry->physicalBlock);
            cache_->evictCacheEntry(entry);
        }
    } else if (L1+L2 >= c) {
        if (L1 + L2 == 2*c) {
            B2.evictLeastRecent();
        }
        replace(physicalBlock);
    }

    CacheEntry* cacheEntry = cache_->loadCacheEntry(access, physicalBlock);
    T1.put(cacheEntry);

if (debug) printf("T1 %lu B1 %lu T2 %lu B2 %lu p %ld\n", T1.size(), B1.size(), T2.size(), B2.size(), p);
}

void
ARC::replace(int64_t physicalBlock)
{
//    if (cache_->getNrUsedBlocks() < cache_->getNrBlocks()) return;

    const int64_t G1 = T1.size() + T2.size()+ B1.size() + B2.size();

    if (T1.size() >= 1
        && ((B2.has(physicalBlock) && T1.size() == p)
            || T1.size() > p))
    {
        CacheEntry* entry = T1.evictLeastRecent();
        B1.put(entry->physicalBlock);
        cache_->evictCacheEntry(entry);
        return;
    } else {
        CacheEntry* entry = T2.evictLeastRecent();
        B2.put(entry->physicalBlock);
        cache_->evictCacheEntry(entry);
    }

    const int64_t G2 = T1.size() + T2.size() + B1.size() + B2.size();
    if (G1 != G2) { printf("break\n"); exit(0); }


}

void
ARC::evict()
{
    replace(-1);
}
