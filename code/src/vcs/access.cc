
#include <fstream>
#include <string>

#include <vcs/access.h>

bool AccessManager::IsMember(const String& username, const String& groupname,
        git_tree *root_tree, git_repository *repo)
{
    int rc;
    String group_key_filename = GetGroupKeyFilename(username, groupname);
    String group_key_dir = GetGroupKeyDir();

    // XXX We may move the following steps to a higher level to make it a bit
    // faster here.
    // Find the sub tree containing group keys
    const git_tree_entry* group_key_tree_entry =
        git_tree_entry_byname(root_tree, group_key_dir.c_str());
    if (group_key_tree_entry == NULL) {
        LOG("The group key dir does not exist!");
        // NOTE Return true anyway in this case because it means that the
        // repository does not support group ownership managerment.
        return true;
    }

    const git_oid* group_key_tree_id = git_tree_entry_id(group_key_tree_entry);
    git_tree* group_key_tree;

    rc = git_tree_lookup(&group_key_tree, repo, group_key_tree_id);
    if (rc < 0) {
        LOG("The group key dir is not a directory!");
        // NOTE Return true anyway in this case because it means that the
        // repository does not support group ownership managerment.
        return true;
    }

    // Check whether the group key exists
    const git_tree_entry* group_key_entry =
        git_tree_entry_byname(group_key_tree, group_key_filename.c_str());
    if (group_key_entry != NULL) {
        return true;
    }

    return false;
}

bool AccessManager::IsMember(const String& username, const String& groupname,
        const String& root_path)
{
    int rc;
    String group_key_filename = GetGroupKeyFilename(username, groupname);
    String group_key_dir = GetGroupKeyDir();
    String absolute_key_path = root_path + group_key_dir + group_key_filename;

    rc = access(absolute_key_path.c_str(), F_OK);
    if (rc < 0) {
        // LOG something
        return false;
    }

    return true;
}

String AccessManager::GetGroupKey(const String& username,
        const String& groupname, const String& root_path)
{
    assert(key_manager_ != NULL);

    int rc;
    String group_key_filename = GetGroupKeyFilename(username, groupname);
    String group_key_dir = GetGroupKeyDir();
    String absolute_key_path = root_path + group_key_dir + group_key_filename;
    std::ifstream key_fin(absolute_key_path.c_str());

    char key_encrypted_content[MAX_KEY_SIZE];
    key_fin.read(key_encrypted_content, MAX_KEY_SIZE);

    String key_content = key_manager_->GetGroupKeyContent(key_encrypted_content);

    return key_content;
}

String AccessManager::GetGroupKeyFilename(const String& username,
        const String& groupname)
{
    // ":" is not allowed in username, so we can use it as the delimeter
    return username + ":" + groupname;
}

String AccessManager::GetGroupKeyDir()
{
    return ".groupkey/";
}

