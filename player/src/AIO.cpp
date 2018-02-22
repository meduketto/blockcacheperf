/*
 * 
 */

#include <iostream>
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <string.h>

#include "AIO.h"

/* Systemcall wrapper for Linux AIO */

static int
io_setup(unsigned nr, aio_context_t* ctxp)
{
    return syscall(__NR_io_setup, nr, ctxp);
}

static int
io_destroy(aio_context_t ctx)
{
    return syscall(__NR_io_destroy, ctx);
}

static int
io_submit(aio_context_t ctx, long nr, struct iocb** iocbpp)
{
    return syscall(__NR_io_submit, ctx, nr, iocbpp);
}

static int
io_getevents(aio_context_t ctx, long min_nr, long max_nr, struct io_event* events, struct timespec* timeout)
{
    return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
}

//


AIO::AIO(const char* devname, int nrEvents, std::size_t sectorSize):
    sectorSize_(sectorSize),
    nrEvents_(nrEvents),
    nrActive_(0)
{
    pageSize_ = getpagesize();

    fd_ = open(devname, O_RDWR | O_DIRECT | O_NOATIME);
    if (fd_ < 0) {
        std::cerr << "Cannot open device." << std::endl;
        exit(1);
    }

    aioCtx_ = 0;
    if (io_setup(nrEvents, &aioCtx_) < 0) {
        std::cerr << "Cannot create AIO context." << std::endl;
        exit(1);
    }

    for (int i = 0; i < nrEvents_; ++i) {
        struct iocb* iocbp = (struct iocb*) malloc(sizeof(struct iocb));
//        posix_memalign((void**) &iocbp, pageSize_, sizeof(struct iocb));
        memset(iocbp, 0, sizeof(struct iocb));
        char* buf;
        posix_memalign((void**) &buf, pageSize_, sectorSize_);
        iocbp->aio_buf = (long long unsigned) buf;
        memset(buf, '@', sectorSize_);
        freeCbs_.push_back(iocbp);
    }

}

AIO::~AIO()
{
    close(fd_);
    fd_ = -1;
    io_destroy(aioCtx_);
    aioCtx_ = 0;
}

void
AIO::processAccess(int64_t sector, bool isWrite)
{
    if (freeCbs_.size() == 0) {
        waitForCompletion();
    }

    struct iocb* iocbp = freeCbs_.front();
    freeCbs_.pop_front();

    iocbp->aio_data = (unsigned long long) iocbp;
    iocbp->aio_lio_opcode = isWrite ? IOCB_CMD_PWRITE : IOCB_CMD_PREAD;
    iocbp->aio_fildes = fd_;
    iocbp->aio_nbytes = sectorSize_;
    iocbp->aio_offset = sector * sectorSize_;

    queuedCbs_.push_back(iocbp);
}

void
AIO::processQueued()
{
    struct iocb* array[nrEvents_ + 1];

    int nr = 0;
    while (queuedCbs_.size() > 0) {
        struct iocb* iocbp = queuedCbs_.front();
        queuedCbs_.pop_front();

        array[nr++] = iocbp;
    }
    array[nr] = nullptr;

    int ret = io_submit(aioCtx_, nr, array);
    if (ret == nr) {
        nrActive_ += nr;
    } else {
        std::cerr << "io_submit error = " << ret << std::endl;
    }
}

void
AIO::waitForCompletion()
{
    if (queuedCbs_.size() >= (std::size_t)(nrEvents_ / 2)) {
        processQueued();
    }

    struct io_event events[nrEvents_];

    int ret = io_getevents(aioCtx_, 1, nrEvents_, &events[0], nullptr);
    if (ret < 0) {
        std::cerr << "io_getevents error = " << ret << std::endl;
        exit(1);
    }

    for (int i = 0; i < ret; ++i) {
        struct iocb* iocbp = (struct iocb*) events[i].data;
        if (events[i].res != (long long) iocbp->aio_nbytes) {
            std::cerr << "error event res=" << events[i].res << ", res2=" << events[i].res2 << std::endl;
        }
        --nrActive_;
        freeCbs_.push_back(iocbp);
    }
}

void
AIO::waitForAllCompleted()
{
    if (queuedCbs_.size() > 0) {
        processQueued();
    }

    while (nrActive_) {
        waitForCompletion();
    }
}
