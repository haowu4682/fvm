// The file describes the Config for a repository

#ifndef SINGLE_BACKTRACE_H_
#define SINGLE_BACKTRACE_H_

// The config for FVM repository
class RepositoryBacktrace {
    public:
        // Constructors
        //
        // Start the backtrace
        int Start();
        // Stop the backtrace
        int Stop();
    private:
        // The relative path for (partial) backtrace mode
        String path_;

        // The config for the repository
        RepoConfig *config_;
};

#endif // SINGLE_BACKTRACE_H_

