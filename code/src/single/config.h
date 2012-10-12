// The file describes the Config for a repository

#ifndef SINGLE_CONFIG_H_
#define SINGLE_CONFIG_H_

#include <map>
#include <string>

#include <common/common.h>

enum TraceLevel {
    kNone = 0,
    kUpdate,
    kFull
};

class TraceLevelManager {
    public:
        // Constructors
        //TraceLevelManager();

        // Get the tracelevel for a specific pathname
        TraceLevel GetTraceLevel(const String& pathname);

        // Add a trace level item to the manager
        void AddTraceLevelItem(const String& pathname, TraceLevel level);

    private:
        Map<String, TraceLevel> trace_level_map_;
};

// The config for FVM repository
class RepoConfig {
    public:
        // Constructors
        RepoConfig();
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

        // Read in a config string and turn it into attributes
        void ReadFromString(const String& config_str);

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

