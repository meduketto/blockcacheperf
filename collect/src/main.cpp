/*
 * 
 */

#include "common.h"
#include "CaptureProcessor.h"

int
main(int argc, char* argv[])
{
    CaptureProcessor processor;

    processor.process(argv[1]);

    processor.printStatistics();

    return 0;
}
