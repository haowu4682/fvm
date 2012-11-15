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

int PartialTracer::Commit()
{
}

int PartialTracer::Commit(const String& branch_name)
{
    int rc;

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

    rc = client_->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return rc;
    }

    rc = client_->Commit(config_.repo_path(), relative_path_, link_dst_, head_id, &config_);
    if (rc < 0) {
        LOG("Cannot checkout the specified repository.");
        return rc;
    }

    return 0;
}

int PartialTracer::Checkout(const String& branch_name)
{
    int rc;

    // Usage: link REPO LINK_SRC LINK_DST
    String head_id;
    rc = client_->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return 0;
    }

    rc = client_->RetrieveHead(config_.repo_path(), branch_name, head_id);
    if (rc < 0) {
        LOG("Cannot retrieve HEAD status.");
        return 0;
    }

    rc = client_->Connect();
    if (rc < 0) {
        LOG("Cannot establish connection to the server.");
        return 0;
    }
    rc = client_->Checkout(config_.repo_path(), relative_path_, link_dst_, head_id);
    if (rc < 0) {
        LOG("Cannot checkout the specified repository.");
        return 0;
    }

    return 0;
}

