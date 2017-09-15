/*
 *
 */

#ifndef ACCESS_SOURCE_H
#define ACCESS_SOURCE_H 1

#include "common.h"
#include "Access.h"

class AccessSource {
public:
    virtual bool nextAccess(Access& access) = 0;
};


#endif /* ACCESS_SOURCE_H */
