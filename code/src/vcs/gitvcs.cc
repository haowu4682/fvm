// The implementation for git api as VCS
//

#include <cstring>
#include <dirent.h>
#include <fstream>
#include <git2.h>
#include <grp.h>
#include <iostream>
#include <map>
#include <pwd.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <backend/backend.h>
#include <common/accessinfo.h>
#include <common/common.h>
#include <common/util.h>
#include <vcs/gitvcs.h>

#define GIT_BINARY "/usr/bin/git"

void PrintOid(const git_oid *oid)
{
    char buf[10000];
    git_oid_tostr(buf, 10000, oid);

    printf("%s\n", buf);
}

String OidToStr(const git_oid *oid)
{
    char buf[10000];
    git_oid_tostr(buf, 10000, oid);

    return buf;
}

void PrintTreeEntry(const git_tree_entry *entry)
{
    const git_oid *id;
    char buf[10000];
    const char *type_name;
    id = git_tree_entry_id(entry);
    git_oid_tostr(buf, 10000, id);
    git_otype type = git_tree_entry_type(entry);
    if (type == GIT_OBJ_BLOB)
        type_name = "blob";
    else if (type == GIT_OBJ_TREE)
        type_name = "tree";
    else
        type_name = "other";

    printf("%s %s %s\n", git_tree_entry_name(entry), buf, type_name);
}

void PrintTree(git_tree *tree)
{
    DBG("tree entry size = %ld", git_tree_entrycount(tree));
    printf("name oid type\n");
    for (int i = 0; i < git_tree_entrycount(tree); ++i) {
        const git_tree_entry* entry = git_tree_entry_byindex(tree, i);
        PrintTreeEntry(entry);
    }
    printf("\n");
}

String GitTreeEntryName::ToString() const
{
    // Use "/" to divide different attributes of a file.
    // Since "/" is forbidden in Linux filenames.
    std::ostringstream sout;

    sout << mode << "@"
         << user << "@"
         << group << "@"
         << name;

    return sout.str();
}

int GitTreeEntryName::FromString(const String& str)
{
    Vector<String> str_array;
    split(str, "@", str_array, 0, 3);

    if (str_array.size() < 4) {
        LOG("tree entry name corrupted: %s", str.c_str());
        return -1;
    }

    mode = atoi(str_array[0].c_str());
    user = str_array[1];
    group = str_array[2];
    name = str_array[3];

    return 0;
}

int GitTreeEntryName::Init(const String& filename, const struct stat* stat)
{
    assert(stat != NULL);

    // Read user name and group name
    struct passwd *password_info = getpwuid(stat->st_uid);
    struct group *group_info = getgrgid(stat->st_gid);

    if (password_info == NULL) {
        LOG("Cannot find the specified user id for %s, uid = %d",
                filename.c_str(), stat->st_uid);
        return -1;
    }

    if (group_info == NULL) {
        LOG("Cannot find the specified group id for %s, gid = %d",
                filename.c_str(), stat->st_gid);
        return -1;
    }

    // Set attributes
    mode = stat->st_mode;
    name = filename;
    user = password_info->pw_name;
    group = group_info->gr_name;
}

int GitTreeEntryName::WriteToFile(const String& filepath) const
{
    // Get uid and gid
    struct passwd *password_info = getpwnam(user.c_str());
    struct group *group_info = getgrnam(group.c_str());

    if (password_info == NULL) {
        LOG("Cannot find the specified user id for %s, username = %s",
                filepath.c_str(), user.c_str());
        return -1;
    }

    if (group_info == NULL) {
        LOG("Cannot find the specified group id for %s, groupname = %s",
                filepath.c_str(), group.c_str());
        return -1;
    }

    // Set mode
    chmod(filepath.c_str(), mode);

    // Set owner
    chown(filepath.c_str(), password_info->pw_uid, group_info->gr_gid);

    return 0;
}

GitTreeEntryName::GitTreeEntryName()
{
    // do nothing
}

GitTreeEntryName::GitTreeEntryName(const String& str)
{
    FromString(str);
}

GitTreeEntryName::GitTreeEntryName(const String& name, const struct stat* stat)
{
    Init(name, stat);
}

enum FileOperation {
    kRead,
    kWrite
};

// Whether a user has permission to access a file
bool HasPermission(const String& username,
        FileOperation op,
        const String& owner,
        const String& group,
        mode_t mode,
        const AccessList& access_list)
{
    assert(op == kRead || op == kWrite);

    if (owner == username) {
        if (op == kRead) {
            if (mode && S_IRUSR) {
                return true;
            }
        } else {
            if (mode && S_IWUSR) {
                return true;
            }
        }
    }

    if (access_list.IsIncluded(username, group)) {
        if (op == kRead) {
            if (mode && S_IRGRP) {
                return true;
            }
        } else {
            if (mode && S_IWGRP) {
                return true;
            }
        }
    }

    if (op == kRead) {
        if (mode && S_IROTH) {
            return true;
        }
    } else {
        if (mode && S_IWOTH) {
            return true;
        }
    }

    return false;
}

bool HasPermission(const String& username,
        FileOperation op,
        const GitTreeEntryName& entry_name,
        const AccessList& access_list)
{
    return HasPermission(username, op, entry_name.user, entry_name.group, entry_name.mode, access_list);
}


// Retrieve the HEAD commit
int RetrieveHeadOid(git_reference* head_ref,
        git_repository* repo,
        git_oid *head_oid_out)
{
    int rc;

    git_oid head_target_oid;
    const git_oid* head_oid;
    char head_name_buf[GIT_OID_HEXSZ+1];
    char *head_name_out;
    const char *head_target_name;

    switch (git_reference_type(head_ref)) {
        case GIT_REF_OID:
            head_oid = git_reference_target(head_ref);
            git_oid_cpy(head_oid_out, head_oid);
            break;

        case GIT_REF_SYMBOLIC:
            head_target_name = git_reference_symbolic_target(head_ref);
            rc = git_reference_name_to_id(&head_target_oid, repo, head_target_name);
            if (rc < 0) {
                LOG("Failure: Cannot retrieve the oid of the HEAD.");
                return rc;
            }

            git_oid_cpy(head_oid_out, &head_target_oid);
            break;

        default:
            LOG("Unsupported git reference type");
            return -1;
    }

    return 0;
}

// Retrieve the HEAD commit
int RetrieveHeadName(git_reference* head_ref,
        git_repository* repo,
        String& head_name)
{
    int rc;

    git_oid head_target_oid;
    const git_oid* head_oid;
    char head_name_buf[GIT_OID_HEXSZ+1];
    char *head_name_out;
    const char *head_target_name;

    switch (git_reference_type(head_ref)) {
        case GIT_REF_OID:
            head_oid = git_reference_target(head_ref);
            head_name_out = git_oid_tostr(head_name_buf, GIT_OID_HEXSZ+1, head_oid);
            head_name = head_name_out;
            break;

        case GIT_REF_SYMBOLIC:
            head_target_name = git_reference_symbolic_target(head_ref);
            rc = git_reference_name_to_id(&head_target_oid, repo, head_target_name);
            if (rc < 0) {
                LOG("Failure: Cannot retrieve the oid of the HEAD.");
                return rc;
            }

            head_name_out = git_oid_tostr(head_name_buf, GIT_OID_HEXSZ+1, &head_target_oid);
            head_name = head_name_out;
            break;

        default:
            LOG("Unsupported git reference type");
            return -1;
    }

    return 0;
}

int GetCommitTree(const git_oid *commit_oid, git_repository *repo, git_tree **commit_tree)
{
    int rc;
    git_commit *commit;

    assert(commit_tree != NULL);

    rc = git_commit_lookup(&commit, repo, commit_oid);
    if (rc < 0) {
        LOG("Failure: Commit object cannot be found.");
        return rc;
    }

    rc = git_commit_tree(commit_tree, commit);
    if (rc < 0) {
        LOG("Failure: Commit tree cannot be found.");
        return rc;
    }

    DBG("commit tree is:");
    PrintTree(*commit_tree);
}

int ChangeListCallback(const char *pathname_c_str, unsigned status, void* list)
{
    Vector<String> *change_list = static_cast<Vector<String> *>(list);

    change_list->push_back(pathname_c_str);
    return 0;
}

const git_tree_entry * GitTreeEntrySearchByName(git_tree *tree, const String& filename)
{
    unsigned tree_size;
    const git_tree_entry *tree_entry;
    const char * tree_entry_name;
    Vector<String> splitted_tree_entry_name;

    tree_size = git_tree_entrycount(tree);

    for (unsigned index = 0; index < tree_size; ++index) {
        tree_entry = git_tree_entry_byindex(tree, index);
        tree_entry_name = git_tree_entry_name(tree_entry);

        split(tree_entry_name, "@", splitted_tree_entry_name);

        if (splitted_tree_entry_name.back() == filename) {
            return tree_entry;
        }
    }

    return NULL;
}

git_filemode_t GitFileMode(mode_t mode)
{
    git_filemode_t git_mode;

    if (S_ISLNK(mode)) {
        git_mode = GIT_FILEMODE_LINK;
    } else if (S_ISDIR(mode)) {
        git_mode = GIT_FILEMODE_TREE;
    } else if (S_ISREG(mode) && (mode & S_IXUSR)) {
        git_mode = GIT_FILEMODE_BLOB_EXECUTABLE;
    } else if (S_ISREG(mode) && (!(mode & S_IXUSR))) {
        git_mode = GIT_FILEMODE_BLOB;
    } else {
        LOG("Unsupported type: %d", mode);
        git_mode = GIT_FILEMODE_NEW;
    }

    return git_mode;
}

int GitTreeSearch(git_oid *relative_oid,
        Vector<git_tree *> *tree_link_path,
        git_repository *repo,
        git_tree *root_tree,
        const String& relative_path)
{
    int rc;
    Vector<String> relative_path_splitted;
    split(relative_path, "/", relative_path_splitted, 1);

    git_tree *current_tree = root_tree;
    const git_tree_entry *tree_entry;

    const git_oid *current_oid;

    current_oid = git_tree_id(root_tree);
    DBG("OID:");
    PrintOid(current_oid);

    for (int i = 0; i < relative_path_splitted.size(); ++i) {
        if (tree_link_path != NULL) {
            tree_link_path->push_back(current_tree);
        }
        PrintTree(current_tree);
        LOG("next str: %s", relative_path_splitted[i].c_str());

        // Step 1: Find the tree entry
        tree_entry = GitTreeEntrySearchByName(current_tree, relative_path_splitted[i]);
        if (tree_entry == NULL) {
            LOG("path name not found: %s", relative_path_splitted[i].c_str());
            return -1;
        }
        PrintTreeEntry(tree_entry);

        // Step 2: Find the object
        current_oid = git_tree_entry_id(tree_entry);
        if (current_oid == NULL) {
            LOG("can not find the object to the entry.");
            return -1;
        }

        DBG("CURRENT OID:");
        PrintOid(current_oid);

        // Step 3: Check if the object is tree object, and modify current_tree
        if (i != relative_path_splitted.size() - 1) {

            rc = git_tree_lookup(&current_tree, repo, current_oid);
            if (rc < 0) {
                LOG("Unknown error while converting git_object to git_tree for pathname: %s",
                        relative_path_splitted[i].c_str());
                return rc;
            }
        }
    }

    if (relative_oid != NULL) {
        git_oid_cpy(relative_oid, current_oid);
    }

    DBG("RELATIVE OID RESULT:");
    PrintOid(relative_oid);

    return 0;
}

int GitBlobWrite(git_repository *repo, git_blob *blob, const String& destination,
        const GitTreeEntryName &name)
{
    const void *buf = git_blob_rawcontent(blob);
    size_t size = git_blob_rawsize(blob);

    DBG("size = %ld", size);
    DBG("content = %s", (char *)buf);

    FILE *file = fopen(destination.c_str(), "w");
    if (file == NULL) {
        LOG("Cannot open file to write: %s", destination.c_str());
        return -1;
    }
    fwrite((char *)buf, size, 1, file);
    fclose(file);
    name.WriteToFile(destination);

    return 0;
}

struct GitTreeWriteCallbackPayload {
    git_repository *repo;
    const String *destination;
//    GitTreeEntryName info;
    AccessList *access_list;
    const String *username;
};

// TODO Remove replication
String GetRealPath(const char *root)
{
    std::ostringstream os;

    // Divide the combine point
    Vector<String> path_list;
    split(root, "/", path_list, 0);

    for (int i = 0; i < path_list.size(); ++i)
    {
        GitTreeEntryName entry(path_list[i]);
        os << entry.name << "/";
    }

    return os.str();
}

int GitTreeWriteCallback(const char *root,
        const git_tree_entry *entry,
        void *payload)
{
    DBG("Tree Entry: ");
    PrintTreeEntry(entry);

    int rc;
    GitTreeWriteCallbackPayload *data =
        static_cast<GitTreeWriteCallbackPayload *> (payload);

    // Step 1 Get FileTreeEntryName
    const char *entry_name_str = git_tree_entry_name(entry);
    GitTreeEntryName tree_entry_name;
    tree_entry_name.FromString(entry_name_str);

    // Step 2 Check access permission
    if (!HasPermission(*data->username, kRead, tree_entry_name, *data->access_list)) {
        return 0;
    }

    // Step 3 Get child path
    std::ostringstream file_name_stream;
    file_name_stream << *data->destination << "/" << GetRealPath(root);
    file_name_stream << tree_entry_name.name;
    String file_name = file_name_stream.str();
    DBG("destination:%s, root:%s, name:%s", data->destination->c_str(),
            root, tree_entry_name.name.c_str());
    DBG("file name:%s", file_name.c_str());

    // Step 4 Create file/dir for child
    git_otype entry_type = git_tree_entry_type(entry);
    const git_oid *entry_oid;
    git_blob *entry_blob;

    switch (entry_type) {
        case GIT_OBJ_TREE:
            mkdir(file_name.c_str(),  0100755);
            tree_entry_name.WriteToFile(file_name);
            break;

        case GIT_OBJ_BLOB:
            entry_oid = git_tree_entry_id(entry);
            rc = git_blob_lookup(&entry_blob, data->repo, entry_oid);
            if (rc < 0) {
                LOG("Error in git data structure: blob is not found.");
            }
            GitBlobWrite(data->repo, entry_blob, file_name, tree_entry_name);
            break;

        default:
            LOG("Unknown type: %d", entry_type);
    }

    return 0;
}

int GitTreeWrite(git_repository *repo, git_tree *tree, const String& destination,
        const String& username, AccessList &list)
{
    int rc;

    struct GitTreeWriteCallbackPayload data;
    data.repo = repo;
    data.destination = &destination;
    data.username = &username;
    data.access_list = &list;

    rc = git_tree_walk(tree, GIT_TREEWALK_PRE, GitTreeWriteCallback, &data);
    if (rc < 0) {
        LOG("Cannot write the files into destination. Errorcode = %d", rc);
        return rc;
    }

    return 0;
}

int GitObjectWrite(git_repository *repo,
        const git_oid *root_oid,
        const String& destination,
        const String& username,
        AccessList &access_list)
{
    git_otype root_type;
    git_object *root_object;
    git_blob *root_blob;
    git_tree *root_tree;
    int rc;

    GitTreeEntryName tree_entry_name;
    struct stat destination_stat;

    rc = git_object_lookup(&root_object, repo, root_oid, GIT_OBJ_ANY);
    if (rc < 0) {
        LOG("Cannot find object!");
        return rc;
    }
    root_type = git_object_type(root_object);

    switch (root_type) {

        case GIT_OBJ_TREE:
            rc = git_tree_lookup(&root_tree, repo, root_oid);
            if (rc < 0) {
                LOG("Error in git data structure: tree is not found.");
            }

            GitTreeWrite(repo, root_tree, destination, username, access_list);
            break;

        case GIT_OBJ_BLOB:
            rc = git_blob_lookup(&root_blob, repo, root_oid);
            if (rc < 0) {
                LOG("Error in git data structure: blob is not found.");
            }

            stat(destination.c_str(), &destination_stat);
            tree_entry_name.Init(destination, &destination_stat);

            GitBlobWrite(repo, root_blob, destination, tree_entry_name);
            break;

        default:
            char buf[10000];
            char *out = git_oid_tostr(buf, 10000, root_oid);
            if (out != NULL) {
                LOG("Unexpected type %d for %s, skipped.", root_type, out);
            } else {
                LOG("Unexpected type %d for unknown object, skipped.", root_type);
            }
    }
    return 0;
}

int CombineObject(git_tree **new_root_tree,
        git_repository *repo,
        git_tree *root_tree,
        const git_oid *combine_object_oid,
        const String& combine_point)
{
    int rc;
    git_oid old_combine_oid;
    Vector<git_tree *> tree_link_path;

    // Divide the combine point
    Vector<String> combine_point_list;
    split(combine_point, "/", combine_point_list, 1);

    // Search for the combine point in the root tree
    rc = GitTreeSearch(&old_combine_oid, &tree_link_path, repo, root_tree,
            combine_point);
    if (rc < 0) {
        LOG("The combine point %s does not exist.", combine_point.c_str());
        return rc;
    }

    DBG("combine point oid:");
    PrintOid(&old_combine_oid);

    // Recursively modify the tree, up till the root
    git_oid current_new_oid;
    git_tree *current_parent_tree;
    git_treebuilder *tree_builder;
    git_oid_cpy(&current_new_oid, combine_object_oid);

    DBG("Current New Oid:");
    PrintOid(&current_new_oid);

    if (combine_point_list.size() != tree_link_path.size()) {
        LOG("Unknown error in GitTreeSearch: combine_point_list_size = %ld, "
                "tree_link_path_size = %ld", combine_point_list.size(), tree_link_path.size());
        return -1;
    }

    for (int i = combine_point_list.size() - 1; i >= 0; --i) {
        DBG("Commit point path: %s", combine_point_list[i].c_str());

        // Find old tree entry
        current_parent_tree = tree_link_path[i];
        String &current_child_name = combine_point_list[i];
        const git_tree_entry* old_tree_entry = GitTreeEntrySearchByName(
                current_parent_tree, current_child_name);
        if (old_tree_entry == NULL) {
            LOG("Old tree entry does not exist! We cannot perfrom the commit action due to that.");
            return -1;
        }

        // Find new tree entry name
        const char *child_entry_name_c_str = git_tree_entry_name(old_tree_entry);

        // Write the tree entry into the tree
        git_treebuilder_create(&tree_builder, current_parent_tree);

        git_treebuilder_insert(NULL, tree_builder, child_entry_name_c_str,
                &current_new_oid, GIT_FILEMODE_TREE);

        git_treebuilder_write(&current_new_oid, repo, tree_builder);

        git_treebuilder_free(tree_builder);
    }

    DBG("Current New Oid:");
    PrintOid(&current_new_oid);

    // Return the final root tree
    rc = git_tree_lookup(new_root_tree, repo, &current_new_oid);
    if (rc < 0) {
        LOG("unknown error while retrieving new commit tree for oid");
        PrintOid(&current_new_oid);
        return rc;
    }

    return 0;
}

void UpdateAccessList(AccessList& list, GitTreeEntryName& entry_name)
{
    list.AddUser(entry_name.user, entry_name.group);
}

// Create an object for the specific file in the path, recursively.
// Return: 0 Object created normally.
//         1 Object not created because it is not included, or not with the name
//           of the branch, or has no permission.
//         <0 Error code.
int CreateObjectRecursive(
        // Output
        git_oid *source_oid,
        struct stat *source_stat,
        AccessList *new_access_list,

        // Input
        git_repository *repo,
        const String& username,
        const String& branch_name,
        const String& source_path,
        const String& relative_path,
        const git_oid *old_object_oid,
        //GitTreeEntryName *old_tree_entry_name,
        AccessList *old_access_list,

        // Operator
        IsIncludeOperator &IsIncluded,
        BranchOperator &GetBranch)
{
    int rc;

    git_oid child_oid;
    struct stat source_stat_buf;
    String source_branch_name;

    git_blob *old_blob;
    git_tree *old_tree;
    const git_oid *old_child_oid;

    source_branch_name = GetBranch(relative_path);

    DBG("path:%s, branch:%s, required branch:%s", relative_path.c_str(),
            source_branch_name.c_str(), branch_name.c_str());

    if (source_stat == NULL) {
        source_stat = &source_stat_buf;
    }

    stat(source_path.c_str(), source_stat);

    if (S_ISREG(source_stat->st_mode)) {

        // Check branch
        if (!IsIncluded(relative_path) || source_branch_name != branch_name) {
            if (old_object_oid != NULL) {
                git_oid_cpy(source_oid, old_object_oid);
                return 0;
            }
            return 1;
        }

#if 0
        // Check access list
        if (old_object_oid != NULL && old_tree_entry_name != NULL) {
            //rc = git_blob_lookup(&old_blob, repo, old_object_id);
            //if (rc == 0) {
            //}
            //

            if (!HasPermission(username, kWrite, *old_tree_entry_name, *old_access_list)) {
                git_oid_cpy(source_oid, old_object_oid);
                return 0;
            }
        }
#endif

        // Create a blob for the file
        rc = git_blob_create_fromdisk(source_oid, repo, source_path.c_str());
        if (rc < 0) {
            LOG("Cannot create a blob for file: %s", source_path.c_str());
            return rc;
        }

        DBG("Created blob, oid = ");
        PrintOid(source_oid);

    } else if (S_ISDIR(source_stat->st_mode)) {
        DIR *directory = opendir(source_path.c_str());
        struct dirent *directory_entry;

        if (directory == NULL) {
            LOG("Cannot open dir: %s", source_path.c_str());
            return -1;
        }

        // Read old tree information
        if (old_object_oid == NULL) {
            old_tree = NULL;
        } else {
            rc = git_tree_lookup(&old_tree, repo, old_object_oid);
            if (rc < 0) {
                old_tree = NULL;
            }
        }

        // Create a tree builder for the directory, use old tree information if
        // possible.
        git_treebuilder *tree_builder;
        // HAO do not include files that have been deleted!
        //git_treebuilder_create(&tree_builder, old_tree);
        git_treebuilder_create(&tree_builder, NULL);

        while ((directory_entry = readdir(directory)) != NULL) {
            // Skip "." and ".."
            //DBG("d_name = %s", directory_entry->d_name);
            if ((!strcmp(directory_entry->d_name, ".")) ||
                    (!strcmp(directory_entry->d_name, "..")))
                continue;

            // Create an object for every child
            String child_absolute_path = source_path + '/' + directory_entry->d_name;
            String child_relative_path = relative_path + directory_entry->d_name + '/';
            struct stat child_stat;
            stat(child_absolute_path.c_str(), &child_stat);

            // Set up tree entry name
            GitTreeEntryName tree_entry_name;
            tree_entry_name.Init(directory_entry->d_name, &child_stat);
            git_filemode_t tree_entry_mode = GitFileMode(tree_entry_name.mode);
            String tree_entry_name_str = tree_entry_name.ToString();

            // Find old tree entry id
            // If old tree does exist
            if (old_tree != NULL) {
                // Search for the old tree entry
                const git_tree_entry *old_tree_entry =
                    git_tree_entry_byname(old_tree, tree_entry_name_str.c_str());

                if (old_tree_entry != NULL) {
                    old_child_oid = git_tree_entry_id(old_tree_entry);
                    // Check access list
                    GitTreeEntryName old_tree_entry_name(git_tree_entry_name(old_tree_entry));
                    if (!HasPermission(username, kWrite, old_tree_entry_name, *old_access_list)) {
                        // XXX ad-hoc fix
                        rc = 0;
                        git_oid_cpy(&child_oid, old_child_oid);
                        return 0;
                    }
                } else {
                    old_child_oid = NULL;
                }
            } else {
                old_child_oid = NULL;
            }


            // Recursively call the function to create an object for the child
            rc = CreateObjectRecursive(&child_oid, &child_stat, new_access_list,
                    repo, username, branch_name, child_absolute_path.c_str(),
                    child_relative_path.c_str(), old_child_oid, old_access_list,
                    IsIncluded, GetBranch);
            if (rc < 0) {
                DBG("Failure in recursively create the object!");
                return rc;
            }

created:
            if (rc == 0) {
                // If the child is created, we shall include it into the tree
                // builder

                // Add the child into the tree builder
                rc = git_treebuilder_insert(NULL, tree_builder,
                        tree_entry_name_str.c_str(), &child_oid, tree_entry_mode);
                if (rc < 0) {
                    LOG("Cannot insert object into tree builder for file: %s, errno = %d", child_absolute_path.c_str(), rc);
                    DBG("tree entry name: %s", tree_entry_name_str.c_str());
                    return -1;
                }

                // Update new access list
                UpdateAccessList(*new_access_list, tree_entry_name);
            } else if (rc == 1) {
                LOG("Inexisted path:%s", child_relative_path.c_str());
            } else {
                LOG("ERROR! UNEXPECTED RETURN VALUE: %d", rc);
                return -1;
            }
        }

        // Write out the tree
        rc = git_treebuilder_write(source_oid, repo, tree_builder);
        if (rc < 0) {
            LOG("Failed to write the tree to the repository.");
            return rc;
        }

        // Free out the internal data
        git_treebuilder_free(tree_builder);
        closedir(directory);

        DBG("Created tree, oid = ");
        PrintOid(source_oid);

    } else {
        // TODO Support symbolic link.
        LOG("We do not support the file type for file: %s.", source_path.c_str());
    }

    return 0;
}

bool IsFastForward(git_repository *repo,
        const String& branch_name,
        const git_oid* head_id)
{
    int rc;
    git_reference *ref;
    git_oid ref_head_id;
    bool equal;

    // Step 1: Find the reference
    rc = git_reference_lookup(&ref, repo, branch_name.c_str());
    if (rc != 0) {
        LOG("Branch is not found for %s", branch_name.c_str());
        return false;
    }

    // Step 2: Find the reference head
    rc = RetrieveHeadOid(ref, repo, &ref_head_id);
    if (rc < 0) {
        LOG("Branch head does not exist for %s", branch_name.c_str());
        return false;
    }

    // Step 3: Check whether head is the same with the given parameter
    equal = git_oid_equal(head_id, &ref_head_id);

    return equal;
}

// Read in the access list
int GitVCS::ReadAccessList(
        // Output
        AccessList& access_list,

        // Input
        git_repository* repo,
        git_tree* root_tree)
{
    int rc;
    git_blob* access_list_file_blob;

    const git_tree_entry* access_list_file_entry = git_tree_entry_byname(
            root_tree, access_list_filename_.c_str());
    if (access_list_file_entry == NULL) {
        return -1;
    }

    const git_oid* access_list_file_id = git_tree_entry_id(
            access_list_file_entry);
    if (access_list_file_id == NULL) {
        return -1;
    }

    rc = git_blob_lookup(&access_list_file_blob, repo, access_list_file_id);
    if (rc < 0) {
        return rc;
    }

    const void* access_list_buf = git_blob_rawcontent(access_list_file_blob);
    git_off_t access_list_bufsize = git_blob_rawsize(access_list_file_blob);
    if (access_list_buf == NULL) {
        // This means that the blob has no content, which means the access list
        // is empty. We shall return safely without adding content in the access
        // list.
        return 0;
    }

    // Read in the access list
    String access_list_str((const char *)access_list_buf, access_list_bufsize);
    access_list.FromString(access_list_str);

    return 0;
}

int GitVCS::WriteAccessList (
        // Output
        git_tree** new_root_tree,

        // Input
        git_repository* repo,
        const AccessList& access_list,
        git_tree* old_root_tree)
{
    int rc;
    git_oid access_list_file_id, new_root_tree_id;
    git_treebuilder* builder;

    git_treebuilder_create(&builder, old_root_tree);

    String access_list_str = access_list.ToString();
    LOG("access_list_str = %s", access_list_str.c_str());
    git_blob_create_frombuffer(&access_list_file_id, repo,
            access_list_str.c_str(), access_list_str.size());

    rc = git_treebuilder_insert(NULL, builder, access_list_filename_.c_str(),
            &access_list_file_id, GIT_FILEMODE_BLOB);
    if (rc < 0) {
        LOG("Cannot insert access list!");
        return rc;
    }

    rc = git_treebuilder_write(&new_root_tree_id, repo, builder);
    if (rc < 0) {
        LOG("Cannot write the tree into the repo!");
        return rc;
    }

    rc = git_tree_lookup(new_root_tree, repo, &new_root_tree_id);
    if (rc < 0) {
        LOG("Cannot find the root tree we just wrote to the repository!");
    }

    git_treebuilder_free(builder);

    return 0;
}

// Get the change list for a specific repository
// Return the number of items to be included in the change list.
int GitVCS::GetChangeList(const String& repo_pathname,
        Vector<String>& change_list)
{
    int rc;
    git_repository *repo;

    // Step 1: Open repository
    //
    rc = git_repository_open(&repo, repo_pathname.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot open repository.");
        return -1;
    }

    // Step 2: Analyze files and add into change list
    git_status_options git_change_list_option;
    git_change_list_option.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    git_change_list_option.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
        GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;
    git_status_foreach_ext(repo, &git_change_list_option, ChangeListCallback, &change_list);

    // Step 3: Free resource
    git_repository_free(repo);

    return 0;
}

// Checkout a commit
int GitVCS::Checkout(const String& repo_pathname,
        const String& commit_id,
        const String& relative_path,
        const String& destination_path,
        const String& username)
{
    int rc;
    git_repository *repo;
    git_tree *commit_tree;
    git_oid relative_oid, commit_oid;
    AccessList access_list;

    // Step 1: Open repository
    //
    rc = git_repository_open(&repo, repo_pathname.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot open repository.");
        return rc;
    }

    // Step 2: Retrieve the commit tree
    //
    rc = git_oid_fromstr(&commit_oid, commit_id.c_str());
    if (rc < 0) {
        LOG("Failure: Commit id cannot be found.");
        return rc;
    }

    rc = GetCommitTree(&commit_oid, repo, &commit_tree);
    if (rc < 0) {
        LOG("Failure: Commit tree cannot be found.");
        return rc;
    }

    // Step 3: Find the root at the relative path
    //

    rc = GitTreeSearch(&relative_oid, NULL, repo, commit_tree, relative_path);

    if (rc < 0) {
        LOG("Failure: Path %s does not exist.", relative_path.c_str());
        return rc;
    }

    DBG("Relative root is");
    PrintOid(&relative_oid);

    // Step 4: Read access list
    rc = ReadAccessList(access_list, repo, commit_tree);
    if (rc < 0) {
        LOG("Failure: Cannot read access list");
        return rc;
    }

    // Step 5: Overwrite files in destination
    rc = GitObjectWrite(repo, &relative_oid, destination_path, username, access_list);
    if (rc < 0) {
        LOG("Failure: Cannot write into %s", destination_path.c_str());
        return rc;
    }

    // Step 6: Free resource
    git_tree_free(commit_tree);
    git_repository_free(repo);

    return 0;
}

// Commit the change into a specific repository
// Return 0 if succeeded, -1 otherwise.
int GitVCS::Commit(const String& repo_pathname,
        const Vector<String>& change_list)
{
    return 0;
}

// Commit the change into a specific repository, using the old commit id as the
// base.
// TODO Pass old commit id as a parameter
int GitVCS::PartialCommit(const String& repo_pathname,
        const String& branch_name,
        const String& relative_path,
        const String& work_dir,
        const String& username,
        IsIncludeOperator &IsIncluded,
        BranchOperator &GetBranch)
{
    int rc;
    git_repository *repo;
    git_signature *author, *committer;

    git_reference *head_ref;
    git_oid partial_oid, old_commit_id, new_commit_id;
    const git_oid *old_commit_tree_id;
    git_oid old_partial_tree_id;
    git_tree *new_commit_tree, *old_commit_tree;
    git_commit *old_commit, *new_commit;

    AccessList old_access_list, new_access_list;

    // TODO Make parameter
    bool CheckFastForward = false;

    // Step 1: Open repository
    rc = git_repository_open(&repo, repo_pathname.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot open repository.");
        return rc;
    }

    // Step 2: Find old commit id
    // Step 2.1 Lookup the reference
    rc = git_branch_lookup(&head_ref, repo, branch_name.c_str(), GIT_BRANCH_LOCAL);
    if (rc < 0) {
        LOG("Failure: Cannot retrieve head reference");
    }

    // Step 2.2 Find the oid
    rc = RetrieveHeadOid(head_ref, repo, &old_commit_id);
    if (rc < 0) {
        LOG("Failure: Cannot retrieve the oid of the HEAD.");
        return rc;
    }

    // Step 3: Get old commit tree
    rc = git_commit_lookup(&old_commit, repo, &old_commit_id);
    if (rc < 0) {
        LOG("Failure: Commit object cannot be found.");
        return rc;
    }

    rc = git_commit_tree(&old_commit_tree, old_commit);
    if (rc < 0) {
        LOG("Failure: Commit tree cannot be found.");
        return rc;
    }

    old_commit_tree_id = git_tree_id(old_commit_tree);

#ifdef DEBUG
    DBG("old commit tree: ");
    PrintOid(git_tree_id(old_commit_tree));
    PrintTree(old_commit_tree);
#endif

    // Step 4: Get old access list
    rc = ReadAccessList(old_access_list, repo, old_commit_tree);
    if (rc < 0) {
        LOG("Failed to read access list!");
        return rc;
    }

    // Step 5: Create partial trees
    // Get old partial tree
    rc = GitTreeSearch(&old_partial_tree_id, NULL, repo, old_commit_tree, relative_path);

    if (rc < 0) {
        LOG("Failure: Path %s does not exist.", relative_path.c_str());
        return rc;
    }

    new_access_list = old_access_list;
    DBG("old_access_list = %s", old_access_list.ToString().c_str());
    DBG("new_access_list = %s", new_access_list.ToString().c_str());

    rc = CreateObjectRecursive(&partial_oid, NULL, &new_access_list, repo, username_,
            branch_name, work_dir, relative_path, &old_partial_tree_id,// NULL,
            &old_access_list, IsIncluded, GetBranch);
    if (rc < 0) {
        LOG("Failed to create the partial tree!");
        return rc;
    } else if (rc == 1) {
        LOG("the specified relative path is not included or not with a correct branch");
    }

    // Debug: print partial tree
#ifdef DEBUG
    git_tree *partial_tree;
    rc = git_tree_lookup(&partial_tree, repo, &partial_oid);
    if (rc < 0) {
        DBG("partial tree is not a tree! oid = ");
        PrintOid(&partial_oid);
    } else {
        DBG("partial tree: ");
        PrintTree(partial_tree);
    }
#endif

    // Step 6: Create new trees
    rc = CombineObject(&new_commit_tree, repo, old_commit_tree, &partial_oid, relative_path);
    if (rc < 0) {
        LOG("Failed to combine old commit with the new one!");
        return rc;
    }

#ifdef DEBUG
    DBG("full commit tree: ");
    PrintOid(git_tree_id(new_commit_tree));
    PrintTree(new_commit_tree);
#endif

    // Step 7: Create new access list
    rc = WriteAccessList(&new_commit_tree, repo, new_access_list, new_commit_tree);
    if (rc < 0) {
        LOG("Failed to read access list!");
        return rc;
    }

    // Step 8: Create author and committer information
    rc = git_signature_now(&author, username_.c_str(), user_email_.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot create the author.");
        return -1;
    }

    rc = git_signature_now(&committer, username_.c_str(), user_email_.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot create the committer.");
        return -1;
    }

    // Step 9: Get branch information
    const char *reference_name_c_str = git_reference_name(head_ref);
    DBG("old reference name: %s", reference_name_c_str);

    // Step 10: Create commit object
    if (CheckFastForward) {
        if (!IsFastForward(repo, branch_name, &old_commit_id)) {
            return kNotFastForward;
        }
    }

    git_commit_create_v(
            &new_commit_id,
            repo,
            reference_name_c_str,
            author,
            committer,
            NULL,
            "",
            new_commit_tree,
            1,
            old_commit);

    // Step 8 Free resource
end:
    git_signature_free(author);
    git_signature_free(committer);
    git_tree_free(old_commit_tree);
    git_tree_free(new_commit_tree);
    git_commit_free(old_commit);
    git_repository_free(repo);

    return 0;
}

// Retrieve the HEAD commit
int GitVCS::GetHead(const String& repo_pathname,
        String& head_out)
{
    int rc;
    git_repository *repo;
    git_reference *head_ref;
    const git_oid *head_oid;
    const char *head_target_name;

    // Step 1: Open repository
    rc = git_repository_open(&repo, repo_pathname.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot open repository.");
        return rc;
    }

    // Step 2: Read head
    rc = git_repository_head(&head_ref, repo);
    if (rc < 0) {
        LOG("Failure: Cannot open repository HEAD.");
        return rc;
    }

    rc = RetrieveHeadName(head_ref, repo, head_out);
    if (rc < 0) {
        LOG("Failure: Cannot retrieve the oid of the HEAD.");
        return rc;
    }

    return 0;
}

// Retrieve the HEAD commit
int GitVCS::GetHead(const String& repo_pathname,
        const String& branch_name,
        String& head_out)
{
    int rc;
    git_repository *repo;
    git_reference *head_ref;
    git_oid head_oid;
    char head_name[GIT_OID_HEXSZ+1];
    char *head_name_out;

    // Step 1: Open repository
    rc = git_repository_open(&repo, repo_pathname.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot open repository.");
        return rc;
    }

    // Step 2: Lookup the reference
    rc = git_branch_lookup(&head_ref, repo, branch_name.c_str(), GIT_BRANCH_LOCAL);
    if (rc < 0) {
        LOG("Failure: Cannot retrieve head reference");
    }

    // Step 3: Retrieve the head oid string
    rc = RetrieveHeadName(head_ref, repo, head_out);
    if (rc < 0) {
        LOG("Failure: Cannot retrieve the oid of the HEAD.");
        return rc;
    }

    return 0;
}

int GitVCS::BranchCreate(const String& repo_name,
        const String& branch_name,
        const String& head_commit_id)
{
    int rc;
    git_repository *repo;
    git_reference *branch;
    git_commit *branch_head_commit;
    git_oid branch_head_oid;

    // Step 1 Open repository
    rc = git_repository_open(&repo, repo_name.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot open repository.");
        return rc;
    }

    // Step 2 Find branch head
    rc = git_oid_fromstr(&branch_head_oid, head_commit_id.c_str());
    if (rc < 0) {
        LOG("Failure: Commit id cannot be found.");
        return rc;
    }

    rc = git_commit_lookup(&branch_head_commit, repo, &branch_head_oid);
    if (rc < 0) {
        LOG("Failure: Commit object cannot be found.");
        return rc;
    }

    // Step 3 Create branch
    rc = git_branch_create(&branch, repo, branch_name.c_str(),
            branch_head_commit, true);
    if (rc < 0) {
        LOG("Failure: Cannot create new branch!");
        return rc;
    }

    return 0;
}

int GitVCS::SendPush(const String& repo_pathname, const String& remote_name)
{
    int rc;
    git_push *push;
    git_repository *repo;
    git_remote *remote;

    // Step 1: Initialize repo and remote
    rc = git_repository_open(&repo, repo_pathname.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot open repository.");
        return rc;
    }

    rc = git_remote_load(&remote, repo, remote_name.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot load remote config.");
        return rc;
    }

    rc = git_remote_set_transport(remote, new FVMTransport());
    if (rc < 0) {
        LOG("Failure: Cannot load remote config.");
        return rc;
    }

    // Step 2: Send the push object
    rc = git_push_new(&push, remote);
    if (rc < 0) {
        LOG("Failure: Cannot initialize the push object.");
        return rc;
    }

    rc = git_push_finish(push);
    if (rc < 0) {
        LOG("Failure: Cannot finish the push object.");
        return rc;
    }

    git_push_free(push);
    git_remote_free(remote);
    git_repository_free(repo);

    return 0;
}

int GitVCS::ReceivePush(git_push *push)
{
    git_repository *repo = git_push_repo(push);

    // Step 1: retrieve the pack
    // git_backbuilder* packbuilder = git_push_packbuilder

    // Step 2: write all objects
    // git_pack_builder_unpack(packbuilder)

    return 0;
}

