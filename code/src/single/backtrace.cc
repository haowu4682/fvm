// The implementation of repository backtrace mode.

#include <string>
#include <vector>

#include <single/backtrace.h>

RepositoryBacktrace::RepositoryBacktrace()
{
    // TODO Implement
}

int RepositoryBacktrace::Start()
{
    if (config_ != NULL) {
        old_trace_level_ = config_->GetTraceLevel(path_);
        config_->SetTraceLevel(path_, kNone);
    }

    return 0;
}

int RepositoryBacktrace::Stop()
{
    if (config_ != NULL) {
        config_->SetTraceLevel(path_, old_trace_level_);
    }

    return 0;
}

int cmd_repo_backtrace(const Vector<String> &args)
{
    if (args.size() < 2) {
        printf("Usage: backtrace path_to_backtrace");
        return -1;
    }

    // TODO Implement

    return 0;
}

