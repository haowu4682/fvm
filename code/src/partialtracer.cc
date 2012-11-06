// The file describes Tracer, the utility to trace files

#include <string>
#include <vector>

#include <partialtracer.h>

#include <common/common.h>
#include <vcs/gitvcs.h>

PartialTracer::PartialTracer(const String& repo, const String& link_src,
        const String& link_dst, FVMClient *client)
{
    relative_path_ = link_src;
    link_dst_ = link_dst;
    config_.repo_path(repo);
    client_ = client;
}

void PartialTracer::PartialCommit()
{
    int rc;

    String head_id;
    rc = client_->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return;
    }

    rc = client_->RetrieveHead(config_.repo_path(), head_id);
    if (rc < 0) {
        LOG("Cannot retrieve HEAD status.");
        return;
    }

    rc = client_->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return;
    }

    rc = client_->Commit(config_.repo_path(), relative_path_, link_dst_, head_id, &config_);
    if (rc < 0) {
        LOG("Cannot checkout the specified repository.");
        return;
    }
}

void PartialTracer::Trace()
{
    if (running_) {
#if 0
        Vector<String> change_list;
        Vector<String> commit_list;
        if (vcs != NULL) {
            vcs->GetChangeList(config_.repo_path(), change_list);

            for (Vector<String>::iterator it = change_list.begin();
                    it != change_list.end(); ++it) {
                if (config_.GetTraceLevel(*it) != kNone) {
                    commit_list.push_back(*it);
                }
            }
        }
#endif
        PartialCommit();
    }

    sleep(config_.time_interval_s());
}

#if 0
int cmd_partial_tracer_start(const Vector<String>& args)
{
    // Step 1: Read in config
    // TODO Read in config

    // Step 2: Initialize tracer
    // TODO <repo, link_src, link_dst>
    Tracer* tracer = new PartialTracer;

    // Step 3: Start the tracer
    tracer->Start();

    return 0;
}
#endif

