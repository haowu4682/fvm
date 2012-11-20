// The implementation of repository backtrace mode.

#include <string>
#include <vector>

#include <common/util.h>
#include <single/branchmanager.h>

const String BranchManager::kDefaultBranchName = "master";

BranchManager::BranchManager() : default_branch_name_(kDefaultBranchName)
{
}

BranchManager::BranchManager(const String& default_branch_name) :
    default_branch_name_(default_branch_name)
{
}

int BranchManager::Add(const String& name, const String& path)
{
    branch_map_[name] = path;
    return 0;
}

int BranchManager::Remove(const String& name)
{
    branch_map_.erase(name);
    return 0;
}

String BranchManager::GetBranch(const String& path) const
{
    String current_path;
    String current_name = default_branch_name_;

    for (Map<String, String>::const_iterator it = branch_map_.begin();
            it != branch_map_.end(); ++it) {
        if (IsPrefix(it->second, path) &&
                !IsPrefix(it->second, current_path)) {
            current_path = it->second;
            current_name = it->first;
        }
    }

    return current_name;
}

