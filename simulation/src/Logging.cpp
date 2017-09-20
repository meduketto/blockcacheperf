/*
 * 
 */

#include <iostream>

#include "Logging.h"

static bool doDebug;

namespace logging {

void
debug(std::function<void(std::stringstream&)> logFunction)
{
    if (doDebug) {
        std::stringstream ss;
        logFunction(ss);
        std::cerr << ss.str() << std::endl;
    }
}

void
warning(std::function<void(std::stringstream&)> logFunction)
{
    std::stringstream ss;
    logFunction(ss);
    std::cerr << "Warning: " << ss.str() << std::endl;
}

void
error(std::function<void(std::stringstream&)> logFunction)
{
    std::stringstream ss;
    logFunction(ss);
    std::cerr << "ERROR: " << ss.str() << std::endl;
    abort();
}

void
setDebug(bool flag)
{
    doDebug = flag;
}

} /* namespace logging */
