// The implementation for git api as VCS

#include <vcs/gitvcs.h>

// Get the change list for a specific repository
// Return the number of items to be included in the change list.
int GitVCS::GetChangeList(const String& repo_pathname,
        Vector<String>& change_list)
{
    return 0;
}

// Commit the change into a specific repository
// Return 0 if succeeded, -1 otherwise.
int GitVCS::Commit(const String& repo_pathname,
        const Vector<String>& change_list)
{
    return 0;
}

