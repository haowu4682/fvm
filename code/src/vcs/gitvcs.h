// The file describes the interface for a Version Control System (VCS)

#ifndef VCS_GITVCS_H_
#define VCS_GITVCS_H_

#include <string>
#include <vector>

#include <common/common.h>
#include <vcs/vcs.h>

// The Interface for a version control system
class GitVCS : public VersionControlSystem {
    public:
        // Constructors
        GitVCS() {}
        virtual ~GitVCS() {}

        // Mutators
        void username(const String &value) { username_ = value; }
        void user_email(const String &value) { user_email_ = value; }


        // Get the change list for a specific repository
        // Return the number of items to be included in the change list.
        virtual int GetChangeList(const String& repo_pathname,
                Vector<String>& change_list);

        // Commit the change into a specific repository
        // Return 0 if succeeded, -1 otherwise.
        virtual int Commit(const String& repo_pathname,
                const Vector<String>& change_list);

        // Checkout a commit
        virtual int Checkout(const String& repo_pathname,
            const String& commit_id,
            const String& relative_path,
            const String& destination_path);

        // Partial commit
        virtual int PartialCommit(const String& repo_pathname,
                const String& old_commit_id,
                const String& relative_path,
                const String& work_dir,
                IsIncludeOperator &IsIncluded);

        // Retrieve the HEAD commit
        virtual int GetHead(const String& repo_pathname,
                const String& branch_name,
                String& head_out);
        virtual int GetHead(const String& repo_pathname,
                String& head_out);

    private:
        String username_;
        String user_email_;
};

#endif // VCS_GITVCS_H_

