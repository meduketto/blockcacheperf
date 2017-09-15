/*
 * 
 */

#ifndef BLOCK_MAP_H
#define BLOCK_MAP_H 1

class BlockMap {
public:
    BlockMap();
    ~BlockMap();
    bool map(const char* filename);

    int64_t getPhysicalPosition(int64_t logicalPosition) const;

private:
    struct fiemap *map_;
};


#endif /* BLOCK_MAP_H */
