// The file describes the Config for a repository

#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_

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
        const Map<String, TraceLevel>* trace_level_map() const {
            return &trace_level_map_;
        }

        // Get the tracelevel for a specific pathname
        TraceLevel GetTraceLevel(const String& pathname) const;

        // Add a trace level item to the manager
        void AddTraceLevelItem(const String& pathname, TraceLevel level);

        // Convert trace level name into value
        //static TraceLevel GetTraceLevelFromString(const String& name);

        // Print out the string
        String ToString() const;

    private:
        Map<String, TraceLevel> trace_level_map_;
};

// The config for FVM repository
class RepoConfig {
    public:
        // Constructors
        RepoConfig();
        RepoConfig(const String& config_str);
        RepoConfig(std::istream& config_stream);

        // Accesstors & Mutators
        long time_interval_s() const { return time_interval_s_; }
        void time_interval(long value) { time_interval_s_ = value; }

        String username() const { return username_; }
        void username(const String& value) { username_ = value; }

        String user_email() const { return user_email_; }
        void user_email(const String& value) { user_email_ = value; }

        String user_account() const { return user_account_; }
        void user_account(const String& value) { user_account_ = value; }

        String pubkey_file() const { return pubkey_file_; }
        void pubkey_file(const String& value) { pubkey_file_ = value; }

        String privkey_file() const { return privkey_file_; }
        void privkey_file(const String& value) { privkey_file_ = value; }

        const Map<String, TraceLevel>* trace_level_map() {
            return trace_level_manager_.trace_level_map();
        }

        // Return the tracelevel of a specific pathname
        TraceLevel GetTraceLevel(const String& pathname) const {
            return trace_level_manager_.GetTraceLevel(pathname);
        }

        // Set the tracelevel for a specific path
        void SetTraceLevel(const String& pathname, TraceLevel value) {
            trace_level_manager_.AddTraceLevelItem(pathname, value);
        }

        // Read in a config string and turn it into attributes
        void ReadFromString(const String& config_str);
        void ReadFromStream(std::istream& config_stream);

        // Print out the string
        String ToString() const;

    private:
        // The time interval for tracing in seconds
        long time_interval_s_;
        // Default values for repo config
        const static long kDefaultTimeInterval = 600;

        // Trace level manager
        TraceLevelManager trace_level_manager_;

        // user name and email
        String username_;
        String user_email_;

        // user account
        String user_account_;

        // pubkey file
        String pubkey_file_;
        // privkey file
        String privkey_file_;
};

#endif // CONFIG_CONFIG_H_

