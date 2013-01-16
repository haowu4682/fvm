// The file describes Tracer, the utility to trace files

#include <string>
#include <vector>

#include <partialtracer.h>

#include <common/common.h>
#include <vcs/gitvcs.h>

PartialTracer::PartialTracer(const String& repo, const String& link_src,
        const String& link_dst, FVMClient *client, RepoConfig *config)
    : Tracer(config)
{
    relative_path_ = link_src;
    link_dst_ = link_dst;
    repo_path_ = repo;
    client_ = client;
}

int PartialTracer::Commit()
{
    const Map<String, String>& branch_map = branch_manager_.branch_map();

    for (Map<String, String>::const_iterator it = branch_map.begin();
            it != branch_map.end(); ++it) {
        Commit(it->first);
    }

    Commit(branch_manager_.default_branch_name());
}

int PartialTracer::Commit(const String& branch_name)
{
    int rc;

#if 0
    String head_id;
    rc = client_->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return rc;
    }

    rc = client_->RetrieveHead(config_.repo_path(), head_id);
    if (rc < 0) {
        LOG("Cannot retrieve HEAD status.");
        return rc;
    }
#endif

    rc = client_->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return rc;
    }

    rc = client_->Commit(repo_path_, relative_path_, link_dst_,
            branch_name, config_, &branch_manager_);
    if (rc < 0) {
        LOG("Cannot commit to the specified repository.");
        return rc;
    }

    return 0;
}

int PartialTracer::Checkout(const String& branch_name)
{
    int rc;

    String head_id;
    rc = client_->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return 0;
    }

    rc = client_->RetrieveHead(repo_path_, branch_name, head_id);
    if (rc < 0) {
        LOG("Cannot retrieve HEAD status.");
        return 0;
    }

    rc = client_->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return 0;
    }

    rc = client_->Checkout(repo_path_, relative_path_, link_dst_, head_id, config_);
    if (rc < 0) {
        LOG("Cannot checkout the specified repository.");
        return 0;
    }

    return 0;
}

int PartialTracer::InitBacktrace(const String& branch_name, const String& commit_id)
{
    int rc;

    rc = client_->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return 0;
    }

    rc = client_->BranchCreate(repo_path_, branch_name, commit_id);
    if (rc < 0) {
        LOG("Cannot create the speicifed branch.");
        return 0;
    }

    return 0;
}

