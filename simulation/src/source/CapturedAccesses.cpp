/*
 * 
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "source/CapturedAccesses.h"

CapturedAccesses::CapturedAccesses():
    file_(nullptr)
{
}

CapturedAccesses::~CapturedAccesses()
{
    if (file_) fclose(file_);
    file_ = nullptr;
}

bool
CapturedAccesses::nextAccess(Access& access)
{
    if (!file_) return false;

    char line[512];

    while (fgets(line, sizeof(line)-1, file_)) {
        if (strncmp(line, "r ", 2) == 0) {
            access.sector = strtoll(line + 2, nullptr, 10);
            access.setRead();
            return true;
        } else if (strncmp(line, "w ", 2) == 0) {
            access.sector = strtoll(line + 2, nullptr, 10);
            access.setWrite();
            return true;
        } else {
            // ignore comments, empty lines, etc
        }
    }

    if (ferror(file_)) {
        fprintf(stderr, "CapturedAccesses::nextAccess() fread error '%s'\n", strerror(errno));
        fclose(file_);
        file_ = nullptr;
        return false;
    }

    fclose(file_);
    file_ = nullptr;
    return false;
}

bool
CapturedAccesses::open(const char* filename)
{
    if (file_) fclose(file_);

    file_ = fopen(filename, "r");
    if (!file_) {
        fprintf(stderr, "CapturedAccesses::open(): cannot open file '%s' error '%s'\n",
                filename, strerror(errno));
        return false;
    }

    return true;
}
