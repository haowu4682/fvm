// Utilities

#include <sstream>

#include <common/util.h>

// Split a string
// Returns: the number of substrings
int split(const String& str, // The source string
        const String& delimeters, // The delimeters
        Vector<String>& str_array,
        int start_index /* = 0 */,
        int max_count /* = INT_MAX */) // Output
{
    int end_index;
    int count = 0;

    if (str.empty()) {
        return 0;
    }

    end_index = str.find_first_of(delimeters, start_index);

    while (end_index != String::npos && count < max_count) {
        str_array.push_back(str.substr(start_index, end_index - start_index));
        ++count;
        start_index = end_index + 1;
        end_index = str.find_first_of(delimeters, start_index);
    }

    if (start_index < str.size()) {
        str_array.push_back(str.substr(start_index));
        ++count;
    }

    return count;
}

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
        return target.compare(0, source.size(), source) == 0;
    }
}

