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
    String current_path;
    TraceLevel current_level = kNone;

    for (Map<String, TraceLevel>::iterator it = trace_level_map_.begin();
            it != trace_level_map_.end(); ++it) {
        if (IsPrefix(it->first, pathname) &&
                !IsPrefix(it->first, current_path)) {
            current_path = it->first;
            current_level = it->second;
        }
    }

    return current_level;
}

void TraceLevelManager::AddTraceLevelItem(const String& pathname, TraceLevel level)
{
    trace_level_map_[pathname] = level;
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

