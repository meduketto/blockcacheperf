/*
 * 
 */

#include <iostream>

#include "Logging.h"

static bool doDebug = false;

static int lastStatusLen = 0;

namespace logging {

void
status(const std::string& str)
{
    if (lastStatusLen) std::cout << '\r';
    std::cout << str;
    for (int i = str.size(); i < lastStatusLen; ++i) std::cout << ' ';
    lastStatusLen = str.size();
}

void
clearStatus()
{
    if (lastStatusLen) {
        std::cout << '\n';
    }
    lastStatusLen = 0;
}

void
debug(std::function<void(std::stringstream&)> logFunction)
{
    clearStatus();

    if (doDebug) {
        std::stringstream ss;
        logFunction(ss);
        std::cerr << ss.str() << std::endl;
    }
}

void
warning(std::function<void(std::stringstream&)> logFunction)
{
    clearStatus();

    std::stringstream ss;
    logFunction(ss);
    std::cerr << "Warning: " << ss.str() << std::endl;
}

void
error(std::function<void(std::stringstream&)> logFunction)
{
    clearStatus();

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
