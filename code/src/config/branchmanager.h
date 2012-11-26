// The file describes the branch manager used in backtrace mode

#ifndef CONFIG_BRANCHMANANGER_H_
#define CONFIG_BRANCHMANANGER_H_

#include <string>

#include <common/common.h>
#include <config/config.h>

class BranchManager {
    public:
        // Constructors
        BranchManager();
        BranchManager(const String& default_branch_name);

        // mutators and accessors
        String default_branch_name() const { return default_branch_name_; }
        void default_branch_name(const String& value) { default_branch_name_ = value; }
        const Map<String, String>& branch_map() const { return branch_map_; }

        // Retrieve the branch name for a specified branch
        String GetBranch(const String& path) const;

        // Add a branch content
        int Add(const String& name, const String& path);

        // Remove a branch content
        int Remove(const String& name);

    private:
        // Constants
        const static String kDefaultBranchName;

        // Members
        String default_branch_name_;
        Map<String, String> branch_map_; // name -> path
};

#endif // CONFIG_BRANCHMANANGER_H_

