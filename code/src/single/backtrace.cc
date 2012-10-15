// The implementation of repository backtrace mode.

#include <single/backtrace.h>

int cmd_repo_backtrace(Vector<String> &args)
{
    if (args.size() < 2) {
        printf("Usage: backtrace path_to_backtrace");
        return -1;
    }

    return 0;
}

