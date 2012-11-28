// The file describes Tracer, the utility to trace files

#include <string>
#include <vector>

#include <common/common.h>
#include <tracer.h>
#include <vcs/gitvcs.h>

Tracer* Tracer::current_tracer = NULL;

Tracer::Tracer()
{
    running_ = false;
    vcs = new GitVCS();

    Init();
}

Tracer::Tracer(RepoConfig *config)
{
    running_ = false;
    vcs = new GitVCS();

    config_ = config;
    Init();
}

void Tracer::Start()
{
    running_ = true;
}

void Tracer::Stop()
{
    running_ = false;
}

// An auxilary function to call tracer->Trace() for pthread.
void * trace_pthread_function(void * tracer_void_ptr)
{
    // WARN: We do not do type check, so the user must make sure that tracer is
    // a pointer to Tracer class.
    Tracer * tracer = static_cast<Tracer *>(tracer_void_ptr);
    tracer->Trace();
}

void Tracer::Init()
{
    int rc;

    if ((rc = pthread_create(&trace_thread_, NULL, &trace_pthread_function, this))) {
        LOG("Thread creation failed: %d\n", rc);
        LOG("The tracer cannot be started!");
    }
}

void Tracer::Trace()
{
    if (running_) {
        Commit();
    }

    sleep(config_->time_interval_s());
}

int Tracer::InitBacktrace(const String& branch_name, const String& commit_id)
{
    vcs->BranchCreate(repo_path_, branch_name, commit_id);
    return 0;
}

// Start backtrace
int Tracer::StartBacktrace(const String& branch_name, const String& pathname)
{
    branch_manager_.Add(branch_name, pathname);
    return 0;
}

// End backtrace
int Tracer::StopBacktrace(const String& branch_name, BacktraceMergeChoice merge_choice)
{
    switch(merge_choice) {
        case kAbandon:
            Abandon(branch_manager_.default_branch_name(), branch_name);
            break;
        case kOverwrite:
            Overwrite(branch_manager_.default_branch_name(), branch_name);
            break;
        case kMerge:
            Merge(branch_manager_.default_branch_name(), branch_name);
            break;
        default:
            LOG("Unknown merge choice: %d", merge_choice);
    }

    branch_manager_.Remove(branch_name);
    return 0;
}

int Tracer::Commit()
{
    // TODO New Implementation
    Vector<String> change_list;
    Vector<String> commit_list;
    if (vcs != NULL) {
        vcs->GetChangeList(repo_path_, change_list);

        for (Vector<String>::iterator it = change_list.begin();
                it != change_list.end(); ++it) {
            if (config_->GetTraceLevel(*it) != kNone) {
                commit_list.push_back(*it);
            }
        }

        vcs->Commit(repo_path_, commit_list);
    }
}

int Tracer::Checkout(const String& branch_name)
{
    // TODO Implement
}

int Tracer::Abandon(const String& old_branch, const String& new_branch)
{
    // XXX Only good when old_branch = master!
    branch_manager_.Remove(new_branch);
    Checkout(new_branch);
    Commit();

    return 0;
}

int Tracer::Overwrite(const String& old_branch, const String& new_branch)
{
    // XXX Only good when old_branch = master!
    branch_manager_.Remove(new_branch);
    Commit();

    return 0;
}

int Tracer::Merge(const String& old_branch, const String& new_branch)
{
    // TODO Implement
    return 0;
}

int cmd_repo_start(const Vector<String>& args)
{
    // Step 1: Read in config
    // TODO Read in config

    // Step 2: Initialize tracer
    if (Tracer::current_tracer == NULL) {
        // TODO Use config here
        Tracer::current_tracer = new Tracer();
    }

    Tracer* tracer = Tracer::current_tracer;

    // Step 3: Start the tracer
    tracer->Start();

    return 0;
}

