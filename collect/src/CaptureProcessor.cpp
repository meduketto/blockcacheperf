/*
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "common.h"
#include "CaptureProcessor.h"
#include "BlockMap.h"

struct FileMap {
    int64_t inode;
    BlockMap blockMap;
};

CaptureProcessor::CaptureProcessor():
    nrFileReads(0),
    totalFileReadSize(0),
    nrFileWrites(0),
    totalFileWriteSize(0),
    nrBlockReads(0),
    totalBlockReadSize(0),
    nrBlockWrites(0),
    totalBlockWriteSize(0)
{
}

bool
CaptureProcessor::process(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "CaptureProcessor::process(): cannot open file '%s' error '%s'\n",
                filename, strerror(errno));
        return false;
    }

    char line[512];

    while (fgets(line, sizeof(line)-1, file)) {
        if (!processLine(line)) {
            fprintf(stderr, "CaptureProcessor::process() parse error on line '%s'\n", line);
            fclose(file);
            return false;
        }
    }

    if (ferror(file)) {
        fprintf(stderr, "CaptureProcessor::process() fread error '%s'\n", strerror(errno));
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

void
CaptureProcessor::printStatistics() const
{
    printf("#\n");
    printf("# nrFileReads = %ld\n", nrFileReads);
    printf("# nrBlockReads = %ld\n", nrBlockReads);
    printf("# totalFileReadSize = %ld\n", totalFileReadSize);
    printf("# totalBlockReadSize = %ld\n", totalBlockReadSize);
    printf("#\n");
    printf("# nrFileWrites = %ld\n", nrFileWrites);
    printf("# nrBlockWrites = %ld\n", nrBlockWrites);
    printf("# totalFileWriteSize = %ld\n", totalFileWriteSize);
    printf("# totalBlockWriteSize = %ld\n", totalBlockWriteSize);
    printf("#\n");
}

static int64_t
getLong(const char* line, const char* tag)
{
    const char* t = strstr(line, tag);
    if (!t) return -1;
    t += strlen(tag);
    return strtoll(t, nullptr, 10);
}

static char*
getName(char* line)
{
    char* t = strstr(line, "name:");
    if (!t) return nullptr;
    t += 5;
    char *s = t + strlen(t) - 1;
    while (1) {
        if (s[0] == '\r' || s[0] == '\n') {
            s[0] = '\0';
            --s;
        } else {
            break;
        }
    }
    return t;
}

struct FileMap*
CaptureProcessor::getFileMap(int64_t inode)
{
    // There aren't many files, linear search is OK.
    for (auto* fm : fileMaps) {
        if (fm->inode == inode) {
            return fm;
        }
    }
    return nullptr;
}

bool
CaptureProcessor::processFileOp(char* data, bool isWrite)
{
    const int64_t inode = getLong(data, "inode:");
    const int64_t pos = getLong(data, "pos:");
    const int64_t size = getLong(data, "size:");
    if (inode == -1 || pos == -1 || size == -1) return false;

    if (isWrite) {
        ++nrFileWrites;
        totalFileWriteSize += size;
    } else {
        ++nrFileReads;
        totalFileReadSize += size;
    }

    const struct FileMap* fileMap = getFileMap(inode);
    if (!fileMap) {
        return true;
        fprintf(stderr, "CaptureProcessor::processFileOp(): cannot find mapping for inode %ld\n", inode);
        return false;
    }

    for (int64_t logicalPosition = pos; logicalPosition <= pos + size; logicalPosition += 512) {
        const int64_t physicalPosition = fileMap->blockMap.getPhysicalPosition(logicalPosition);
        const int64_t sector = physicalPosition >> 9;
        printf("%s %ld\n", isWrite ? "w" : "r", sector);
    }

    return true;
}

bool
CaptureProcessor::processBlockOp(char* data, bool isWrite)
{
    const int64_t pos = getLong(data, "sector:");
    const int64_t size = getLong(data, "size:");
    if (pos == -1 || size == -1) return false;

    if (isWrite) {
        ++nrBlockWrites;
        totalBlockWriteSize += size;
    } else {
        ++nrBlockReads;
        totalBlockReadSize += size;
    }

    return true;
}

bool
CaptureProcessor::processLine(char* line)
{
    if (strncmp(line, "of ", 3) == 0) {
        const int64_t inode = getLong(line + 3, "inode:");
        if (getFileMap(inode)) {
            // already know the file
            return true;
        }
        const char* name = getName(line + 3);
        if (!name) {
            fprintf(stderr, "Cannot find file name [%s]\n", line);
        }
        struct FileMap *map = new struct FileMap;
        map->inode = inode;
        if (!map->blockMap.map(name)) {
            delete map;
            fprintf(stderr, "Ignoring [%s]\n", name);
            return true;
        }
        fileMaps.push_back(map);

    } else if (strncmp(line, "rf ", 3) == 0) {
        if (!processFileOp(line + 3, false)) return false;

    } else if (strncmp(line, "wf ", 3) == 0) {
        if (!processFileOp(line + 3, true)) return false;

    } else if (strncmp(line, "rb ", 3) == 0) {
        if (!processBlockOp(line + 3, false)) return false;

    } else if (strncmp(line, "wb ", 3) == 0) {
        if (!processBlockOp(line + 3, true)) return false;

    } else {
        // ignore comments, empty lines etc
    }
    return true;
}
