/*
 * 
 */

#include <string.h>

#include "eviction/Algorithms.h"
#include "eviction/ARC.h"
#include "eviction/Clock.h"
#include "eviction/LRU.h"
#include "eviction/Random2.h"

static EvictionAlgorithm*
getLRU()
{
    return new LRU();
}

static EvictionAlgorithm*
getARC()
{
    return new ARC();
}

static EvictionAlgorithm*
getR2()
{
    return new Random2();
}

static EvictionAlgorithm*
getClock()
{
    return new Clock();
}

struct AlgorithmDescription evictionAlgorithms[] = {
    { "ARC",   "Adaptive replacement cache", getARC },
    { "CLOCK", "Clock",                      getClock },
    { "LRU",   "Least recently used",        getLRU },
    { "R2",    "Random-2 choice",            getR2 },
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
