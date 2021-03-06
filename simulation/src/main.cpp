/*
 *
 */

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "common.h"
#include "Simulator.h"
#include "Cache.h"

#include "source/CapturedAccesses.h"
#include "eviction/Algorithms.h"

static struct option longopts[] = {
    { "blocksize", required_argument, 0, 'b' },
    { "nrblocks", required_argument, 0, 'n' },
    { "algorithm", required_argument, 0, 'a' },
    { "output", required_argument, 0, 'o' },
    { "verbose", 0, 0, 'v' },
    { "help", 0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static char shortopts[] = "b:n:a:o:vh";

static void
usage()
{
    std::cout << "Usage: blockcachesim [OPTIONS] inputfile\n"
                 "\n"
                 "where OPTIONS are:\n"
                 " -b, --blocksize=BYTES    Size of the cache blocks (default 4Kb)\n"
                 " -n, --nrblocks=COUNT     Number of cache slots (default 10000)\n"
                 " -a, --algorithm=ALGO     Eviction algorithm (default LRU)\n"
                 " -o, --output=FILE        Post-cache trace output file\n"
                 " -v, --verbose            Enable debug output\n"
                 "\n"
                 "where ALGO is\n";
//             " all                      Everything below\n"

    for (int i = 0; evictionAlgorithms[i].name; ++i) {
        const char* name = evictionAlgorithms[i].name;
        std::cout << ' ' << name;
        for (int j = strlen(name); j < 25; ++j) {
            std::cout << ' ';
        }
        std::cout << evictionAlgorithms[i].description << '\n';
    }
}

static EvictionAlgorithm*
getAlgorithm(const char* name)
{
    AlgorithmDescription* algo = findEvictionAlgorithm(name);
    if (algo) {
        return algo->factory();
    }
    return nullptr;
}

int
main(int argc, char* argv[])
{
    int64_t blockSize = 4096;
    int64_t nrBlocks = 10000;
    EvictionAlgorithm* algorithm = nullptr;
    const char* output = nullptr;
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

            case 'o':
                output = optarg;
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
        algorithm = getAlgorithm("lru");
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
    if (output) cache.setOutputFile(output);

    simulator.setCache(&cache);

    simulator.run();

    return 0;
}
