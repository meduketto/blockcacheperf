/*
 * 
 */

#include <string.h>

#include "eviction/Algorithms.h"
#include "eviction/ARC.h"
#include "eviction/Clock.h"
#include "eviction/ClockPro.h"
#include "eviction/2Q.h"
#include "eviction/LRU.h"
#include "eviction/Random2.h"

static EvictionAlgorithm*
getARC()
{
    return new ARC();
}

static EvictionAlgorithm*
getClock()
{
    return new Clock();
}

static EvictionAlgorithm*
getClockPro()
{
    return new ClockPro();
}

static EvictionAlgorithm*
get2Q()
{
    return new TwoQ();
}

static EvictionAlgorithm*
getLRU()
{
    return new LRU();
}

static EvictionAlgorithm*
getR2()
{
    return new Random2();
}


struct AlgorithmDescription evictionAlgorithms[] = {
    { "ARC",      "Adaptive replacement cache", getARC },
    { "CLOCK",    "Clock",                      getClock },
    { "CLOCKPRO", "Clock-Pro",                  getClockPro },
    { "2Q",       "2Q",                         get2Q },
    { "LRU",      "Least recently used",        getLRU },
    { "R2",       "Random-2 choice",            getR2 },
    { nullptr, nullptr, nullptr }
};

AlgorithmDescription*
findEvictionAlgorithm(const char* name)
{
    for (int i = 0; evictionAlgorithms[i].name; ++i) {
        if (strcasecmp(evictionAlgorithms[i].name, name) == 0) {
            return &evictionAlgorithms[i];
        }
    }
    return nullptr;
}
