/*
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <linux/fiemap.h>

#include "common.h"
#include "BlockMap.h"

BlockMap::BlockMap():
    map_(nullptr)
{
}

BlockMap::~BlockMap()
{
    if (map_) free(map_);
    map_ = nullptr;
}

bool
BlockMap::map(const char* filename)
{
    if (map_) free(map_);
    map_ = nullptr;

    const int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "BlockMap::map(): open '%s' failed\n", filename);
        return false;
    }

    struct fiemap temp;
    memset(&temp, 0, sizeof(struct fiemap));

    temp.fm_start = 0;
    temp.fm_length = FIEMAP_MAX_OFFSET;
    temp.fm_flags = 0;
    temp.fm_extent_count = 0;
    temp.fm_mapped_extents = 0;

    if (ioctl(fd, FS_IOC_FIEMAP, &temp) < 0) {
        fprintf(stderr, "BlockMap::map(): fiemap ioctl failed\n");
        close(fd);
        return false;
    }

    const int extentsSize = sizeof(struct fiemap_extent) * temp.fm_mapped_extents;
    const int size = sizeof(struct fiemap) + extentsSize;

    map_ = (struct fiemap*) malloc(size);
    if (!map_) {
        fprintf(stderr, "BlockMap()::map: Out of memory\n");
        close(fd);
        return false;
    }
    memset(map_, 0, size);

    map_->fm_start = 0;
    map_->fm_length = FIEMAP_MAX_OFFSET;
    map_->fm_flags = 0;
    map_->fm_extent_count = temp.fm_mapped_extents;
    map_->fm_mapped_extents = 0;

    if (ioctl(fd, FS_IOC_FIEMAP, map_) < 0) {
        fprintf(stderr, "BlockMap::map(): second fiemap ioctl failed\n");
        close(fd);
        free(map_);
        map_ = nullptr;
        return false;
    }

    close(fd);

    return true;
}

int64_t
BlockMap::getPhysicalPosition(int64_t logicalPosition) const
{
    if (!map_) return -1;

    for (unsigned int i = 0; i < map_->fm_mapped_extents; ++i) {
        const struct fiemap_extent* extent = &map_->fm_extents[i];
//printf("log %llu phys %llu len %llu\n", extent->fe_logical, extent->fe_physical, extent->fe_length);
        const uint64_t pos = (uint64_t) logicalPosition;
        if (pos >= extent->fe_logical
            && pos < extent->fe_logical + extent->fe_length) {
                const uint64_t physPos = (pos - extent->fe_logical) + extent->fe_physical;
                return (int64_t) physPos;
        }
    }

    return -1;
}
