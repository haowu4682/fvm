// The implementation of repository backtrace mode.

#include <string>
#include <vector>

#include <single/backtrace.h>

int cmd_repo_backtrace(const Vector<String> &args)
{
    if (args.size() < 2) {
        printf("Usage: backtrace path_to_backtrace");
        return -1;
    }

    return 0;
}

