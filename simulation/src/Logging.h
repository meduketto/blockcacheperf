/*
 * 
 */

#ifndef LOGGING_H
#define LOGGING_H 1

#include <functional>
#include <sstream>

namespace logging {

void status(const std::string& ss);

void clearStatus();

void debug(std::function<void(std::stringstream&)> logFunction);

void warning(std::function<void(std::stringstream&)> logFunction);

void error(std::function<void(std::stringstream&)> logFunction);

void setDebug(bool flag);

}

#endif /* LOGGING_H */
