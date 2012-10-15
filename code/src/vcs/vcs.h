// The file describes the interface for a Version Control System (VCS)

#ifndef VCS_VCS_H_
#define VCS_VCS_H_

#include <string>
#include <vector>

#include <common/common.h>

enum FileStatus {
    kUnModified,
    kModified,
    kDeleted,
    kNew,
    kUntracked
}

// The Interface for a version control system
class VersionControlSystem {
    public:
        virtual ~VersionControlSystem() {}

        // Get the change list for a specific repository
        // Return the number of items to be included in the change list.
        virtual int GetChangeList(const String& repo_pathname,
                Vector<String>& change_list) = 0;

        // Commit the change into a specific repository
        // Return 0 if succeeded, -1 otherwise.
        virtual int Commit(const String& repo_pathname,
                const Vector<String>& change_list) = 0;

        // Checkout a commit
        virtual int Checkout(const String& repo_pathname,
                const String& commit_id) = 0;
};

#endif // VCS_VCS_H_

