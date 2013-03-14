#ifndef VCS_ACCESS_H__
#define VCS_ACCESS_H__

#include <string>

#include <git2.h>

#include <common/common.h>
#include <encrypt/encryptionmanager.h>
#include <encrypt/keymanager.h>

class AccessManager {
    public:

        AccessManager(KeyManager *key_manager) : key_manager_(key_manager) {}

        // Get whether a user belongs to a group based on whether the user has
        // the group key based on a root tree of a commit.
        // @param username The user's name
        // @param groupname The group's name
        // @param root_tree The root tree of a commit.
        // @param repo The git repository containing the root tree.
        // @return TRUE if the user has the group key, FALSE otherwise.
        bool IsMember(const String& username, const String& groupname,
                git_tree* root_tree, git_repository *repo);

        // Get whether a user belongs to a group based on whether the user has
        // the group key based on working directory files on the disk.
        // @param username The user's name
        // @param groupname The group's name
        // @param root_path The absolute path of the working directory
        // @return TRUE if the user has the group key, FALSE otherwise.
        bool IsMember(const String& username, const String& groupname,
                const String& root_path);

        // Get the group key path relative to the repository root
        // @param username The user's name
        // @param groupname The group's name
        // @return The relative path.
        virtual String GetGroupKeyFilename(const String& username,
                const String& groupname);

        // Get the directory name containing the group keys.
        // @return The directory
        virtual String GetGroupKeyDir();

    protected:
        KeyManager *key_manager_;
//        EncryptionManager *encryption_manager_;
};

#endif // VCS_ACCESS_H__

