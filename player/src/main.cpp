/*
 *
 */

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include "common.h"

#include "Timer.h"
#include "AIO.h"

static struct option longopts[] = {
    { "repeat", required_argument, 0, 'r' },
    { "queue-size", required_argument, 0, 'q' },
    { "sector-size", required_argument, 0, 's' },
    { "verbose", 0, 0, 'v' },
    { "help", 0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static char shortopts[] = "q:s:r:vh";

static void
usage()
{
    std::cout << "Usage: cacheplayer [OPTIONS] devfile inputfile\n"
                 "\n"
                 "where OPTIONS are:\n"
                 " -r, --repeat <N>         Repeat the test N times (default 1)\n"
                 " -q, --queue_size <N>     AIO queue length (default " << AIO::NR_EVENTS << ")\n"
                 " -s, --sector_size <N>    Disk sector size (default " << AIO::SECTOR_SIZE << ")\n"
                 " -v, --verbose            Enable debug output\n"
                 ;
}

int
main(int argc, char* argv[])
{
    int c;
    int i;
    int repeat = 1;
    std::size_t queue_size = AIO::NR_EVENTS;
    std::size_t sector_size = AIO::SECTOR_SIZE;

    while ((c = getopt_long (argc, argv, shortopts, longopts, &i)) != -1) {
        switch (c) {
            case 'v':
                break;

            case 'h':
                usage();
                return 0;

            case 'r':
                repeat = strtoll(optarg, nullptr, 10);
                break;

            case 'q':
                queue_size = strtoll(optarg, nullptr, 10);
                break;

            case 's':
                sector_size = strtoll(optarg, nullptr, 10);
                break;
        }
    }

    Timer timer;

    AIO aio(argv[optind++], queue_size, sector_size);

    for (int i = 0; i < repeat; ++i) {

        FILE* f = fopen(argv[optind], "r");
        if (!f) {
            std::cerr << "Cannot open input file." << std::endl;
            exit(1);
        }

        char line[512];

        while (fgets(line, sizeof(line)-1, f)) {
            if (strncmp(line, "r ", 2) == 0) {
                int64_t sector = strtoll(line + 2, nullptr, 10);
                aio.processAccess(sector, 0);
            } else if (strncmp(line, "w ", 2) == 0) {
                int64_t sector = strtoll(line + 2, nullptr, 10);
                aio.processAccess(sector, 1);
            } else {
                // ignore comments, empty lines, etc
            }
        }

        fclose(f);
    }

    aio.waitForAllCompleted();

    std::cout << "Completion time = " << timer.stop() << " nanoseconds." << std::endl;

    return 0;
}
