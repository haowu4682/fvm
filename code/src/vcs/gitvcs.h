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

        // Get the change list for a specific repository
        // Return the number of items to be included in the change list.
        virtual int GetChangeList(const String& repo_pathname,
                Vector<String>& change_list);

        // Commit the change into a specific repository
        // Return 0 if succeeded, -1 otherwise.
        virtual int Commit(const String& repo_pathname,
                const Vector<String>& change_list);

};

#endif // VCS_GITVCS_H_

