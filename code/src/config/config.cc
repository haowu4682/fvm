// The file describes utilities for configuration of FVM single user mode

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include <common/util.h>
#include <config/config.h>

// The default constructor for repo config
RepoConfig::RepoConfig()
{
    time_interval_s_ = kDefaultTimeInterval;
}

RepoConfig::RepoConfig(const String& config_str)
{
    RepoConfig();

    ReadFromString(config_str);
}

RepoConfig::RepoConfig(std::istream& config_stream)
{
    RepoConfig();

    ReadFromStream(config_stream);
}

TraceLevel TraceLevelManager::GetTraceLevel(const String& pathname) const
{
    String current_path = "";
    TraceLevel current_level = kNone;

    for (Map<String, TraceLevel>::const_iterator it = trace_level_map_.begin();
            it != trace_level_map_.end(); ++it) {
        DBG("Current path:%s, current level:%d, list path:%s, list level:%d, pathname:%s",
                current_path.c_str(), current_level,
                it->first.c_str(), it->second, pathname.c_str());
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

    ReadFromStream(config_stream);
}


void RepoConfig::ReadFromStream(std::istream& config_stream)
{
    while(config_stream) {
        Vector<String> config_line;
        readline(config_stream, config_line);

        if (config_line[0] == "time_interval") {
            time_interval_s_ = atoi(config_line[1].c_str());
        } else if (config_line[0] == "tracelevel") {
            trace_level_manager_.AddTraceLevelItem(config_line[1], TraceLevel(atoi(config_line[2].c_str())));
        } else if (config_line[0] == "username") {
            username_ = config_line[1];
        } else if (config_line[0] == "user_email") {
            user_email_ = config_line[1];
        } else if (config_line[0] == "user_account") {
            user_account_ = config_line[1];
        } else {
            LOG("Unknown config item: %s", config_line[0].c_str());
        }
    }
}

String TraceLevelManager::ToString() const
{
    std::ostringstream str_stream;

    for (Map<String, TraceLevel>::const_iterator it = trace_level_map_.begin();
            it != trace_level_map_.end(); ++it) {
        str_stream << "key: " << it->first << "\t"
                   << "value:" << it->second
                   << std::endl;
    }

    return str_stream.str();
}

String RepoConfig::ToString() const
{
    std::ostringstream str_stream;

    str_stream << "time interval: " << time_interval_s_ << std::endl;
    str_stream << "user name: " << username_ << std::endl;
    str_stream << "user email: " << user_email_ << std::endl;
    str_stream << "trace level map:" << std::endl;
    str_stream << trace_level_manager_.ToString();

    return str_stream.str();
}

