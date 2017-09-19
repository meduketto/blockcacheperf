/*
 *
 */

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "Simulator.h"
#include "Cache.h"

#include "source/CapturedAccesses.h"
#include "eviction/LRU.h"
#include "eviction/ARC.h"

static struct option longopts[] = {
    { "blocksize", required_argument, 0, 'b' },
    { "nrblocks", required_argument, 0, 'n' },
    { "algorithm", required_argument, 0, 'a' },
    { "verbose", 0, 0, 'v' },
    { "help", 0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static char shortopts[] = "b:n:a:vh";

static void
usage()
{
    puts("Usage: blockcachesim [OPTIONS] inputfile\n"
         " -b, --blocksize=BYTES    Size of the cache blocks (4Kb)\n"
         " -n, --nrblocks=COUNT     Number of cache slots (10000)\n"
         " -a, --algorithm=ALGO     Eviction algorithm\n"
         " -v, --verbose            Debug output\n"
         "\n"
         "where ALGO is\n"
         " LRU                      Least recently used\n"
         " ARC                      Adaptive replacement cache\n"
    );
}

static EvictionAlgorithm*
getAlgorithm(const char* name)
{
    if (strcasecmp(name, "lru") == 0) return new LRU();
    if (strcasecmp(name, "arc") == 0) return new ARC();
    return nullptr;
}

int
main(int argc, char* argv[])
{
    int64_t blockSize = 4096;
    int64_t nrBlocks = 10000;
    EvictionAlgorithm* algorithm = nullptr;
    int c;
    int i;

    while ((c = getopt_long (argc, argv, shortopts, longopts, &i)) != -1) {
        switch (c) {
            case 'b':
                blockSize = strtoll(optarg, nullptr, 10);
                break;

            case 'n':
                nrBlocks = strtoll(optarg, nullptr, 10);
                break;

            case 'a':
                algorithm = getAlgorithm(optarg);
                break;

            case 'v':
                logging::setDebug(true);
                break;

            case 'h':
                usage();
                return 0;
        }
    }

    if (!algorithm) {
        printf("Using LRU\n");
        algorithm = new LRU();
    }

    Simulator simulator;
    CapturedAccesses captured;

    if (optind >= argc) {
        fprintf(stderr, "main: need input file\n");
        return 1;
    }

    if (!captured.open(argv[optind])) {
        return 1;
    }

    simulator.setAccessSource(&captured);

    Cache cache(blockSize, nrBlocks, algorithm);

    simulator.setCache(&cache);

    simulator.run();

    return 0;
}
