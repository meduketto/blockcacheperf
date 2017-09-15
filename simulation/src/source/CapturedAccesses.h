/*
 *
 */

#ifndef CAPTURED_ACCESSES_H
#define CAPTURED_ACCESSES_H 1

#include <stdio.h>

#include "common.h"
#include "AccessSource.h"

class CapturedAccesses: public AccessSource {
public:
    CapturedAccesses();
    ~CapturedAccesses();
    virtual bool nextAccess(Access& access);

    bool open(const char* filename);

private:
    FILE* file_;
};



#endif /* CAPTURED_ACCESSES_H */
