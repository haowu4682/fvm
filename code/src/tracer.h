// The file describers Tracer, the utility to trace files

#ifndef TRACER_H__
#define TRACER_H__

#include <pthread.h>

#include <common/common.h>
#include <config/branchmanager.h>
#include <config/config.h>
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
        Tracer(RepoConfig *config);

        // Initialize the tracer thread
        virtual void Init();

        // Start the tracer
        virtual void Start();
        // Stop the tracer
        virtual void Stop();

        // Manually make a commit
        virtual int Commit();

        // Manually checks out the repository
        virtual int Checkout(const String& branch_name);

        // Init a backtrace
        virtual int InitBacktrace(const String& branch_name, const String& commit_id);

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
        RepoConfig *config_;

        // The friend function is used for pthread only
        friend void * trace_pthread_function(void * /* tracer_void_ptr */);

        // The branch manager
        BranchManager branch_manager_;

        // The version control system
        VersionControlSystem* vcs;

        // Repository path
        String repo_path_;

        // Folloing are some functions to be override by children
        // Trace selected files using the config in config_
        virtual void Trace();
        virtual int Abandon(const String& old_branch, const String& new_branch);
        virtual int Overwrite(const String& old_branch, const String& new_branch);
        virtual int Merge(const String& old_branch, const String& new_branch);
};

#endif // TRACER_H__

