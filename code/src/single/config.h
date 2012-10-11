// The file describes the Config for a repository

#ifndef SINGLE_CONFIG_H_
#define SINGLE_CONFIG_H_

#include <string>

#include <common/common.h>

enum TraceLevel {
    kNone = 0,
    kUpdate,
    kFull
};

class TraceLevelManager {
    // TODO: Implement
    public:
        // Constructors
        //TraceLevelManager();

        // Get the tracelevel for a specific pathname
        TraceLevel GetTraceLevel(const String& pathname);

    private:

};

// The config for FVM repository
class RepoConfig {
    public:
        // Constructors
        RepoConfig();
        // TODO Implement
        RepoConfig(const String& config_str);

        // Accesstors & Mutators
        long time_interval_s() { return time_interval_s_; }
        void time_interval(long value) { time_interval_s_ = value; }
        String repo_path() { return repo_path_; }
        void repo_path(const String& value) { repo_path_ = value; }

        // Return the tracelevel of a specific pathname
        TraceLevel GetTraceLevel(const String& pathname) {
            return trace_level_manager_.GetTraceLevel(pathname);
        }

    private:
        // The time interval for tracing in seconds
        long time_interval_s_;
        // Default values for repo config
        const static long kDefaultTimeInterval = 600;

        // Trace level manager
        TraceLevelManager trace_level_manager_;

        // Repository path
        String repo_path_;
};

#endif // SINGLE_CONFIG_H_

