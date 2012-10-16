// Utilities for programming

#ifndef COMMON_UTIL_H__
#define COMMON_UTIL_H__

#include <iostream>
#include <string>
#include <vector>

#include <common/common.h>

// Read a line and split it into string arrays
std::istream& readline(std::istream& is, // the input stream for the command
        Vector<String>& str_array); // the string array to store the result

// Is source a prefix of the target?
bool IsPrefix(const String& source, const String& target);

#endif // COMMON_UTIL_H__

