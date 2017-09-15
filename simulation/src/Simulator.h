/*
 *
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H 1

#include "common.h"

class AccessSource;
class Cache;

class Simulator {
public:
    Simulator();
    void setAccessSource(AccessSource* accessSource);
    void setCache(Cache* cache);
    void run();

private:
    AccessSource* accessSource_;
    Cache* cache_;
};


#endif /* SIMULATOR_H */
