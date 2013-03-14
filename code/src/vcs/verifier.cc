
#include <deque>
#include <vector>

#include <vcs/gitvcs.h>
#include <vcs/verifier.h>

extern int RetrieveHeadOid(git_reference* head_ref, git_repository *repo,
        git_oid* head_oid_out);
#if 0
{
    int rc;

    git_oid head_target_oid;
    const git_oid* head_oid;
    const char *head_target_name;

    switch (git_reference_type(head_ref)) {
        case GIT_REF_OID:
            head_oid = git_reference_target(head_ref);
            git_oid_cpy(head_oid_out, head_oid);
            break;

        case GIT_REF_SYMBOLIC:
            head_target_name = git_reference_symbolic_target(head_ref);
            rc = git_reference_name_to_id(&head_target_oid, repo, head_target_name);
            if (rc < 0) {
                LOG("Failure: Cannot retrieve the oid of the HEAD.");
                return rc;
            }

            git_oid_cpy(head_oid_out, &head_target_oid);
            break;

        default:
            LOG("Unsupported git reference type");
            return -1;
    }
}

void RetrieveHead(git_repository *repo, )
{
    rc = git_branch_lookup(&head_ref_name, repo, branch_name.c_str(),
            GIT_BRANCH_LOCAL);
    if (rc < 0) {
        LOG("Failure: Cannot retrieve head ref_nameerence");
    }

    // Step 3: Retrieve the head oid string
    rc = RetrieveHeadName(head_ref_name, repo, head_out);
    if (rc < 0) {
        LOG("Failure: Cannot retrieve the oid of the HEAD.");
        return rc;
    }

}
#endif

// Check if A is an ancestor of B. If so, Store all the nodes between A and B in
// the stack.
bool CheckAndCollectAncestor(git_commit* A, git_commit* B,
        Deque<git_commit*>* commit_path)
{
    int rc;
    git_commit* B_parent;

    assert(commit_path != NULL);
    commit_path->push_front(B);

    // Step 1: Check if A == B
    if (git_oid_equal(git_commit_id(A), git_commit_id(B))) {
        return true;
    }

    // Step 2: For each parent, recursively call the function to check for
    // the parent's ancestancy.
    for (int i = 0; i < git_commit_parentcount(B); ++i) {
        rc = git_commit_parent(&B_parent, B, i);
        if (rc < 0) {
            LOG("unknown error when retrieving the %d-th parent.", i);
            continue;
        }

        if (CheckAndCollectAncestor(A, B_parent, commit_path)) {
            return true;
        }

        commit_path->pop_front();
    }

    return false;
}

size_t Verifier::RetrieveCommitList(Deque<git_commit*> &commit_list,
        git_reference *old_ref, git_reference *new_ref, git_repository *repo)
{
    int rc;
    git_oid old_oid, new_oid;
    git_commit *old_commit, *new_commit;

    RetrieveHeadOid(old_ref, repo, &old_oid);
    RetrieveHeadOid(new_ref, repo, &new_oid);

    rc = git_commit_lookup(&old_commit, repo, &old_oid);
    if (rc < 0) {
        LOG("Old commit does not exist!");
        return 0;
    }

    rc = git_commit_lookup(&new_commit, repo, &new_oid);
    if (rc < 0) {
        LOG("New commit does not exist!");
        return 0;
    }

    CheckAndCollectAncestor(old_commit, new_commit, &commit_list);
    return 0;
}

bool Verifier::VerifyTree(git_tree *old_tree, git_tree *new_tree,
        git_repository *repo)
{
    // For each tree entry in old tree
    for (size_t i = 0; i < git_tree_entrycount(old_tree); ++i) {
        // If the tree entry shall not be written by the user
        const git_tree_entry* entry = git_tree_entry_byindex(old_tree, i);
        GitTreeEntryName entry_name(git_tree_entry_name(entry));

        if (!access_manager_->IsMember(entry_name.user, entry_name.group,
                    old_tree, repo)) {
            // TODO Check if the tree entry remain unchanged
        }

        // Verify child trees recursively
        if (git_tree_entry_type(entry) == GIT_OBJ_TREE) {
            //git_tree *old_child_tree
            //git_tree *new_child_tree
            //TODO
        }
    }
}

bool Verifier::VerifyCommit(git_commit *old_commit, git_commit *new_commit,
        git_repository *repo)
{
    int rc;
    git_tree *old_tree, *new_tree;

    // Step 1: Get commit tree
    rc = git_commit_tree(&old_tree, old_commit);
    if (rc < 0) {
        LOG("Cannot retrieve old commit tree!");
    }

    rc = git_commit_tree(&new_tree, new_commit);
    if (rc < 0) {
        LOG("Cannot retrieve new commit tree!");
    }

    // Step 2: For each item in the commit tree, verify permission.
    return VerifyTree(old_tree, new_tree, repo);
}

void Verifier::VerifyPush(git_push *push)
{
    int rc;
    bool flag;

    git_repository *repo;
    git_reference *old_ref, *new_ref;
    git_commit *parent_commit, *child_commit;
    repo = git_push_repo(push);

    // Step 1: Decode ref_name from push
    // This step is done by a hack in libgit2.
    // An item as ref_name represents a reference NAME of a 1-1 matching.
    // Only support 1-1 matching right now.

    // Step 2: For each ref_name, verify that all nodes along the path does not
    // violate access control permission.
    for (int i = 0; i < git_push_refspec_count(push); ++i) {
        Deque<git_commit*> commit_list;
        git_push_spec* ref_spec = git_push_refspec_byindex(push, i);

        // Step 2.1: Find old and new ref_nameerence HEAD
        // The old reference HEAD
        rc = git_branch_lookup(&old_ref, repo, ref_spec->lref,
                GIT_BRANCH_LOCAL);
        if (rc < 0) {
            LOG("Failure: Cannot retrieve head reference");
            continue;
        }

        // The new reference HEAD
        rc = git_branch_lookup(&new_ref, repo, ref_spec->rref,
                GIT_BRANCH_LOCAL);
        if (rc < 0) {
            LOG("Failure: Cannot retrieve head reference");
            continue;
        }

        // Step 2.2: Check whether fast forward
        rc = RetrieveCommitList(commit_list, old_ref, new_ref, repo);
        if (rc < 0) {
            LOG("unknown error in RetrieveCommitList: %d", rc);
            continue;
        }
        if (rc == 0) {
            LOG("new_ref is not a fast forward of old_ref");
            continue;
        }

        // Step 2.3: If fast forward, for each commit object between the old and
        //          the new one, verify the commit object.

        for (Deque<git_commit*>::iterator parent_it = commit_list.begin(),
                child_it = ++commit_list.begin();
                child_it != commit_list.end() && parent_it != commit_list.end();
                ++parent_it, ++child_it) {

            // Check whether child does not violate permission specified by
            // parent.
            flag = VerifyCommit(*parent_it, *child_it, repo);
            if (!flag) {
                LOG("child commit violates permission!");
                continue;
            }

            parent_commit = child_commit;
        }

        // Step 2.4 Return SUCCESS for the refspec and update the reference.
        // TODO implement
    }
}

String Verifier::VerifyPackage(const char* buf, size_t size)
{
    // TODO Implement
    return "";
}

