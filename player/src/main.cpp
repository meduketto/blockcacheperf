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
    { "verbose", 0, 0, 'v' },
    { "help", 0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static char shortopts[] = "vh";

static void
usage()
{
    std::cout << "Usage: cacheplayer [OPTIONS] devfile inputfile\n"
                 "\n"
                 "where OPTIONS are:\n"
                 " -v, --verbose            Enable debug output\n"
                 ;
}

int
main(int argc, char* argv[])
{
    int c;
    int i;

    while ((c = getopt_long (argc, argv, shortopts, longopts, &i)) != -1) {
        switch (c) {
            case 'v':
                break;

            case 'h':
                usage();
                return 0;
        }
    }

    Timer timer;

    AIO aio(argv[1]);

    FILE* f = fopen(argv[2], "r");
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

    aio.waitForCompletion();

    std::cout << "Completion time = " << timer.stop() << " nanoseconds." << std::endl;

    return 0;
}
