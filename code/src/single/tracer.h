// The file describers Tracer, the utility to trace files

#ifndef SINGLE_TRACER_H__
#define SINGLE_TRACER_H__

#include <pthread.h>

#include <common/common.h>
#include <single/branchmanager.h>
#include <single/config.h>
#include <vcs/vcs.h>

enum BacktraceMergeChoice {
    kAbandon,
    kOverwrite,
    kMerge
};

class Tracer {
    public:
        // The current running tracer in the system
        static Tracer *current_tracer;

        // Constructor(s)
        Tracer();
        Tracer(const RepoConfig &config);
        Tracer(const String& config_str);

        // Initialize the tracer thread
        virtual void Init();

        // Start the tracer
        virtual void Start();
        // Stop the tracer
        virtual void Stop();

        // Manually make a commit
        virtual int Commit();

        // Start backtrace
        virtual int StartBacktrace(const String& branch_name, const String& pathname);
        // End backtrace
        virtual int StopBacktrace(const String& branch_name, BacktraceMergeChoice merge_choice);

    protected:
        // Whether the tracing is running.
        bool running_;

        // The thread for tracing
        pthread_t trace_thread_;

        // The config of the tracer
        RepoConfig config_;

        // The friend function is used for pthread only
        friend void * trace_pthread_function(void * /* tracer_void_ptr */);

        // The branch manager
        BranchManager branch_manager_;

        // The version control system
        VersionControlSystem* vcs;

        // Folloing are some functions to be override by children
        // Trace selected files using the config in config_
        virtual void Trace();
        virtual int Checkout(const String& branch_name);
        virtual int Abandon(const String& old_branch, const String& new_branch);
        virtual int Overwrite(const String& old_branch, const String& new_branch);
        virtual int Merge(const String& old_branch, const String& new_branch);
};

#endif // SINGLE_TRACER_H__

