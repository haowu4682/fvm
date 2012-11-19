// The file describes the interface for a Version Control System (VCS)

#ifndef VCS_GITVCS_H_
#define VCS_GITVCS_H_

#include <string>
#include <sys/stat.h>
#include <vector>

#include <common/common.h>
#include <vcs/vcs.h>

struct GitTreeEntryName {
    mode_t mode;
    String user;
    String group;
    String name;

    String ToString() const;
    int FromString(const String& str);
    int Init(const String& name, const struct stat* stat);
    int WriteToFile(const String& filepath) const;

    GitTreeEntryName();
    GitTreeEntryName(const String& str);
    GitTreeEntryName(const String& name, const struct stat* stat);
};

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
                const String& relative_path,
                const String& work_dir,
                IsIncludeOperator &IsIncluded,
                BranchOperator &GetBranch);

        // Retrieve the HEAD commit
        virtual int GetHead(const String& repo_pathname,
                const String& branch_name,
                String& head_out);
        virtual int GetHead(const String& repo_pathname,
                String& head_out);

        // Create a branch
        virtual int BranchCreate(const String& repo_name,
                const String& branch_name,
                const String& head_commit_id);

    private:
        String username_;
        String user_email_;
};

#endif // VCS_GITVCS_H_

