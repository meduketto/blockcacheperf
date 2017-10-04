/*
 * 
 */

#ifndef TIMER_H
#define TIMER_H

#include <time.h>

#include "common.h"

inline int64_t
secondsToNanos(int64_t seconds)
{
    return seconds * 1000L * 1000L * 1000L;
}

inline int64_t
timespecToNanos(const struct timespec& ts)
{
    return secondsToNanos(ts.tv_sec) + ts.tv_nsec;
}

inline int64_t
getClock()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return timespecToNanos(ts);
}

class Timer {
public:
    Timer() { start(); }
    void start() { start_ = getClock(); }
    int64_t stop() { return getClock() - start_; }

private:
    int64_t start_;
};

#endif /* TIMER_H */
