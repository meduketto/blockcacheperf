/*
 * 
 */

#include <stdio.h>

#include "Simulator.h"
#include "AccessSource.h"
#include "Access.h"
#include "Cache.h"

Simulator::Simulator():
    accessSource_(nullptr),
    cache_(nullptr)
{
}

void
Simulator::setAccessSource(AccessSource* accessSource)
{
    accessSource_ = accessSource;
}

void
Simulator::setCache(Cache* cache)
{
    cache_ = cache;
}

void
Simulator::run()
{
    Access access;
    int64_t nr = 0;

    while (accessSource_->nextAccess(access)) {
        cache_->handleAccess(access);
        ++nr;
        if ((nr % 50000) == 0) {
            printf("nr %ld\n", nr);
        }
    }

    cache_->printStatistics();
}
