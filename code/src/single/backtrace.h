// The file describes the Config for a repository

#ifndef SINGLE_BACKTRACE_H_
#define SINGLE_BACKTRACE_H_

#include <string>

#include <common/common.h>
#include <single/config.h>

// The config for FVM repository
class RepositoryBacktrace {
    public:
        // Constructors
        RepositoryBacktrace();

        // Start the backtrace
        int Start();
        // Stop the backtrace
        int Stop();
    private:
        // The relative path for (partial) backtrace mode
        String path_;

        // The old trace level for the path
        TraceLevel old_trace_level_;

        // The config for the repository
        RepoConfig *config_;
};

#endif // SINGLE_BACKTRACE_H_

