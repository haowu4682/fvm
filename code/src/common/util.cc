// Utilities

#include <sstream>

#include <common/util.h>

// Read a line and split it into string arrays
std::istream& readline(std::istream& is, // the input stream for the command
        Vector<String>& str_array) // the string array to store the result
{
    String line;
    getline(is, line);

    std::istringstream line_stream(line);
    while (line_stream)
    {
        String str;
        line_stream >> str;
        str_array.push_back(str);
    }

    return is;
}

bool IsPrefix(const String& source, const String& target)
{
    if (source.size() > target.size()) {
        return false;
    } else {
        return target.compare(0, source.size(), source);
    }
}

