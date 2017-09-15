/*
 * 
 */

#ifndef CAPTURE_PROCESSOR_H
#define CAPTURE_PROCESSOR_H 1

#include <vector>

class CaptureProcessor {
public:
    CaptureProcessor();
    bool process(const char* filename);
    void printStatistics() const;

private:
    bool processLine(char* line);
    bool processFileOp(char* data, bool isWrite);
    bool processBlockOp(char* data, bool isWrite);
    struct FileMap* getFileMap(int64_t inode);

    std::vector<struct FileMap*> fileMaps;

    int64_t nrFileReads;
    int64_t totalFileReadSize;
    int64_t nrFileWrites;
    int64_t totalFileWriteSize;

    int64_t nrBlockReads;
    int64_t totalBlockReadSize;
    int64_t nrBlockWrites;
    int64_t totalBlockWriteSize;
};

#endif /* CAPTURE_PROCESSOR_H */
