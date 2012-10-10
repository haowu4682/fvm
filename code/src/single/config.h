// The file describes the Config for a repository

#ifndef SINGLE_CONFIG_H_
#define SINGLE_CONFIG_H_

#include <string>

#include <common/common.h>

// The config for FVM repository
class RepoConfig {
    public:
        // Constructors
        RepoConfig();
        // TODO Implement
        RepoConfig(const String& config_str);

    private:
        // The time interval for tracing in seconds
        long time_interval_s_;

        // Default values for repo config
        const static long kDefaultTimeInterval = 600;
};

#endif // SINGLE_CONFIG_H_

