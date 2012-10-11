// The file describes utilities for configuration of FVM single user mode

#include <single/config.h>

// The default constructor for repo config
RepoConfig::RepoConfig()
{
    time_interval_s_ = kDefaultTimeInterval;
    repo_path_ = ".";
}

// TODO: Implement reading config files

