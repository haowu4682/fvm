
#include <cstdio>
#include <fstream>
#include <string>

#include <vcs/access.h>

bool AccessManager::IsMember(const String& username, const String& groupname,
        git_tree *root_tree, git_repository *repo)
{
    int rc;
    String group_key_filename = GetGroupKeyFilename(username, groupname);
    String group_key_dir = GetGroupKeyDir();

    // Find the sub tree containing group keys
    const git_tree_entry *group_key_tree_entry =
        GitTreeEntrySearchByName(root_tree, group_key_dir);
    //const git_tree_entry* group_key_tree_entry =
    //    git_tree_entry_byname(root_tree, group_key_dir.c_str());
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
    //const git_tree_entry* group_key_entry =
    //    git_tree_entry_byname(group_key_tree, group_key_filename.c_str());
    const git_tree_entry *group_key_entry =
        GitTreeEntrySearchByName(group_key_tree, group_key_filename);
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
        const String& groupname, git_tree* root_tree,
        git_repository *repo)
{
    int rc;
    String group_key_filename = GetGroupKeyFilename(username, groupname);
    String group_key_dir = GetGroupKeyDirName();

    // Find the sub tree containing group keys
    const git_tree_entry *group_key_tree_entry =
        GitTreeEntrySearchByName(root_tree, group_key_dir);
    //DBG("Git Tree ID:");
    PrintOid(git_tree_id(root_tree));

    if (group_key_tree_entry == NULL) {
        LOG("The group key dir %s does not exist!", group_key_dir.c_str());
        return "";
    }

    const git_oid* group_key_tree_id = git_tree_entry_id(group_key_tree_entry);
    git_tree* group_key_tree;

    rc = git_tree_lookup(&group_key_tree, repo, group_key_tree_id);
    if (rc < 0) {
        LOG("The group key dir is not a directory!");
        // NOTE Return true anyway in this case because it means that the
        // repository does not support group ownership managerment.
        return "";
    }

    // Check whether the group key exists
    //const git_tree_entry* group_key_entry =
    //    git_tree_entry_byname(group_key_tree, group_key_filename.c_str());
    const git_tree_entry *group_key_entry =
        GitTreeEntrySearchByName(group_key_tree, group_key_filename);
    if (group_key_entry == NULL) {
        LOG("No group key exists!");
        return "";
    }

    const git_oid* group_key_id = git_tree_entry_id(group_key_entry);
    git_blob* group_key_blob;

    rc = git_blob_lookup(&group_key_blob, repo, group_key_id);
    if (rc < 0) {
        LOG("The group key file is not a regular file!");
        return "";
    }

    String key_encrypted_content((char *)git_blob_rawcontent(group_key_blob),
            git_blob_rawsize(group_key_blob));
    String key_content = key_manager_->GetGroupKeyContent(key_encrypted_content);

    return key_content;
}

String AccessManager::GetGroupKey(const String& username,
        const String& groupname, const String& root_path)
{
    assert(key_manager_ != NULL);

    int rc;
    String group_key_filename = GetGroupKeyFilename(username, groupname);
    String group_key_dir = GetGroupKeyDir();

    String absolute_key_path;
    if (root_path.size() == 0 || root_path[root_path.size()-1] == '/') {
        absolute_key_path = root_path + group_key_dir + group_key_filename;
    } else {
        absolute_key_path = root_path + "/" + group_key_dir + group_key_filename;
    }

    std::ifstream key_fin(absolute_key_path.c_str());

    char key_encrypted_content[MAX_KEY_SIZE];
    key_fin.read(key_encrypted_content, MAX_KEY_SIZE);

    String key_content = key_manager_->GetGroupKeyContent(String(key_encrypted_content, key_fin.gcount()));

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

String AccessManager::GetGroupKeyDirName()
{
    return ".groupkey";
}

bool AccessManager::AddGroupKey(const String& old_username, const String& new_username,
        const String& groupname, const String& user_pubkey, const String& root_path)
{
    assert(key_manager_ != NULL);

    int rc;
    String group_key_content = AccessManager::GetGroupKey(
        old_username, groupname, root_path);

    if (group_key_content.size() == 0) {
        // The group key does not exist, we need to create a new one.
        size_t key_size = encryption_manager_->GetKeySize();
        group_key_content.resize(key_size);
        for (int i = 0; i < key_size; ++i) {
            // XXX Magic number
            group_key_content[i] = rand() % (128-32) + 32;
        }
    }

    LOG("group_key_content = %s", group_key_content.c_str());

    String group_key_filename = GetGroupKeyFilename(new_username, groupname);
    String group_key_dir = GetGroupKeyDir();

    String absolute_key_path = root_path + group_key_dir + group_key_filename;
    std::ofstream key_fout(absolute_key_path.c_str());

    String key_encrypted_content = key_manager_->
        GetNewGroupKeyFileContent(group_key_content);

    key_fout.write(key_encrypted_content.c_str(), key_encrypted_content.size());

    return true;
}

bool AccessManager::RemoveGroupKey(const String& username,
        const String& groupname, const String& root_path)
{
    String group_key_filename = GetGroupKeyFilename(username, groupname);
    String group_key_dir = GetGroupKeyDir();

    String absolute_key_path = root_path + group_key_dir + group_key_filename;

    std::remove(absolute_key_path.c_str());
}

