#ifndef VCS_VERIFIER__
#define VCS_VERIFIER__

#include <git2.h>

#include <common/common.h>
#include <deque>

#include <vcs/access.h>

class Verifier {
    public:
        Verifier(AccessManager *access_manager)
            : access_manager_(access_manager) {}

        // Verify the whole push package
        // buf: the incoming package
        // size: the incoming package size
        // ret: the response
        String VerifyPackage(const char* buf, size_t size);

    protected:
        // Returns TRUE if the commit pair does not violate permission
        // i.e. the user does not modify any files in old_commit which he has
        // no permission in the new_commit.
        // @param old_commit
        // @param new_commit
        // @param repo The parent repository
        // @return whether the user obeys the permissions. TRUE if no violation,
        //      FALSE otherwise.
        bool VerifyCommit(git_commit *old_commit, git_commit *new_commit,
                git_repository *repo);

        // Returns TRUE if the tree pair does not violate permission
        // i.e. the user does not modify any files in old_tree which he has
        // no permission in the new_tree.
        // @param old_tree
        // @param new_tree
        // @param repo The parent repository
        // @return whether the user obeys the permissions. TRUE if no violation,
        //      FALSE otherwise.
        bool VerifyTree(git_tree *old_tree, git_tree *new_tree,
                git_repository *repo);

        // Verify a push object
        void VerifyPush(git_push *push/*,  git_repository *repo */ );

        // Return a list of commit id from old ref to new ref (both included).
        // @param commit_id_list The list of commit id in the path. empty if
        //      new_ref is not a fast forward of old_ref.
        // @param old_ref The old ref.
        // @param new_ref The new ref.
        // @return count of commit_id_list. 0 if old_ref is not a fast forward
        //      of new_ref.
        size_t RetrieveCommitList(Deque<git_commit*> &commit_list,
                git_reference *old_ref, git_reference *new_ref,
                git_repository *repo);

    private:
        AccessManager *access_manager_;
};

#endif // VCS_VERIFIER__

