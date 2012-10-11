// The file describes utilities for configuration of FVM single user mode

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include <common/util.h>
#include <single/config.h>

// The default constructor for repo config
RepoConfig::RepoConfig()
{
    time_interval_s_ = kDefaultTimeInterval;
    repo_path_ = ".";
}

RepoConfig::RepoConfig(const String& config_str)
{
    RepoConfig();

    ReadFromString(config_str);
}

TraceLevel TraceLevelManager::GetTraceLevel(const String& pathname)
{
    // TODO Implement
    return kNone;
}

void TraceLevelManager::AddTraceLevelItem(const String& pathname, TraceLevel level)
{
    // TODO Implement
}

void RepoConfig::ReadFromString(const String& config_str)
{
    std::istringstream config_stream(config_str);

    while(config_stream) {
        Vector<String> config_line;
        readline(config_stream, config_line);

        if (config_line[0] == "time_interval") {
            time_interval_s_ = atoi(config_line[1].c_str());
        } else if (config_line[0] == "repo_path") {
            repo_path_ = config_line[1];
        } else if (config_line[0] == "tracelevel") {
            trace_level_manager_.AddTraceLevelItem(config_line[1], TraceLevel(atoi(config_line[2].c_str())));
        } else {
            LOG("Unknown config item: %s", config_line[0].c_str());
        }
    }
}

