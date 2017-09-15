/*
 *
 */

#ifndef ACCESS_H
#define ACCESS_H 1

#include "common.h"

class Access {
    static const int IS_WRITE = 1;

public:
    Access():
        sector(0),
        flags(0)
    {
    }

    int64_t sector;
    unsigned int flags;

    bool isWrite() const {
        return flags & IS_WRITE;
    }

    bool isRead() const {
        return !isWrite();
    }

    void setWrite() {
        flags |= IS_WRITE;
    }

    void setRead() {
        flags &= ~IS_WRITE;
    }
};


#endif /* ACCESS_H */
