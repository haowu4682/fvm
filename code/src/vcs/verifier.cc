
#include <vcs/verifier.h>

bool Verifier::VerifyTree(git_tree *tree)
{
    // TODO Implement
    return true;
}

void Verifier::VerifyPush(git_push *push)
{
    // An item in refspec_list represents the reference NAME of a 1-1 matching.
    //XXX Only support 1-1 matching right now.
    Vector<String> refspec_list;

    git_repository *repo;
    /*repo = push->repo TODO hack libgit2*/

    // Step 1: Decode refspec from push
    //TODO Implement (required hacking libgit2)

    // Step 2: For each refspec, verify that all nodes along the path does not
    // violate access control permission.
    for (Vector<String>::iterator it = refspec_list.begin();
            it != refspec_list.end(); ++it) {
        String& refspec = *it;

        // Step 2.1: Find old and new reference HEAD
        // The old reference HEAD
        //vcs->GetHEAD(repo, refspec, old_head);

        // TODO: The new reference HEAD
        // Step 2.2: Check whether fast forward (hacking libgit2)
        // Step 2.3: If fast forward, for each commit object between the old and
        //          the new one, verify the commit object.
    }
}

String Verifier::VerifyPackage(const char* buf, size_t size)
{
    // TODO Implement
    return "";
}

