/*
 *
 */

#ifndef ALGORITHMS_H
#define ALGORITHMS_H 1

#include "common.h"

class EvictionAlgorithm;

struct AlgorithmDescription {
    const char* name;
    const char* description;
    EvictionAlgorithm* (*factory)();
};

extern struct AlgorithmDescription evictionAlgorithms[];

AlgorithmDescription* findEvictionAlgorithm(const char* name);


#endif /* ALGORITHMS_H */
