/*
 * 
 */

#ifndef AIO_H
#define AIO_H 1

#include <list>
#include <linux/aio_abi.h>

#include "common.h"

class AIO {
public:
    static constexpr int NR_EVENTS = 64;
    static constexpr std::size_t SECTOR_SIZE = 512;

    AIO(const char* devname, int nrEvents = NR_EVENTS, std::size_t sectorSize = SECTOR_SIZE);
    ~AIO();
    void processAccess(int64_t sector, bool isWrite);
    void processQueued();
    void waitForCompletion();
    void waitForAllCompleted();

private:
    std::size_t pageSize_;
    std::size_t sectorSize_;
    int fd_;
    int nrEvents_;
    aio_context_t aioCtx_;
    std::list<struct iocb*> queuedCbs_;
    std::list<struct iocb*> freeCbs_;
    int nrActive_;
};


#endif /* AIO_H */
