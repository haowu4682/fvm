// The implementation for git api as VCS
//

#include <cstring>
#include <dirent.h>
#include <fstream>
#include <git2.h>
#include <grp.h>
#include <iostream>
#include <pwd.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

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
    split(str, "@", str_array, 3);

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

int ChangeListCallback(const char *pathname_c_str, unsigned status, void* list)
{
    Vector<String> *change_list = static_cast<Vector<String> *>(list);

    change_list->push_back(pathname_c_str);
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
    return 0;
}

const git_tree_entry * GitTreeEntrySearchByName(git_tree *tree, const String& filename)
{
    // TODO implement
    //
    unsigned tree_size;
    const git_tree_entry *tree_entry;
    const char * tree_entry_name;
    Vector<String> splitted_tree_entry_name;

    tree_size = git_tree_entrycount(tree);

    for (unsigned index = 0; index < tree_size; ++index) {
        tree_entry = git_tree_entry_byindex(tree, index);
        tree_entry_name = git_tree_entry_name(tree_entry);

        split(tree_entry_name, "/", splitted_tree_entry_name);

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
    split(relative_path, "/", relative_path_splitted);

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

    FILE *file = fopen(destination.c_str(), "w");
    fwrite((char *)buf, size, 1, file);
    fclose(file);
    name.WriteToFile(destination);

    return 0;
}

struct GitTreeWriteCallbackPayload {
    git_repository *repo;
    const String *destination;
//    GitTreeEntryName info;
};

int GitTreeWriteCallback(const char *root,
        const git_tree_entry *entry,
        void *payload)
{
    PrintTreeEntry(entry);

    int rc;
    GitTreeWriteCallbackPayload *data =
        static_cast<GitTreeWriteCallbackPayload *> (payload);

    // Step 1 Get FileTreeEntryName
    const char *entry_name_str = git_tree_entry_name(entry);
    GitTreeEntryName tree_entry_name;
    tree_entry_name.FromString(entry_name_str);

    // Step 2 Get child path
    std::ostringstream file_name_stream;
    file_name_stream << *data->destination << "/" << root;
    file_name_stream << "/" << tree_entry_name.name;
    String file_name = file_name_stream.str();

    // Step 3 Create file/dir for child
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

int GitTreeWrite(git_repository *repo, git_tree *tree, const String& destination)
{
    int rc;

    struct GitTreeWriteCallbackPayload data;
    data.repo = repo;
    data.destination = &destination;
//    data.info = name;
    rc = git_tree_walk(tree, GitTreeWriteCallback, GIT_TREEWALK_PRE, &data);
    if (rc < 0) {
        LOG("Cannot write the files into destination. Errorcode = %d", rc);
        return rc;
    }

    return 0;
}

int GitObjectWrite(git_repository *repo, const git_oid *root_oid, const String& destination)
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

            GitTreeWrite(repo, root_tree, destination);
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
    split(combine_point, "/", combine_point_list);

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

        current_parent_tree = tree_link_path[i];
        String &current_child_name = combine_point_list[i];

        git_treebuilder_create(&tree_builder, current_parent_tree);

        git_treebuilder_insert(NULL, tree_builder, current_child_name.c_str(),
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

int CreateObjectRecursive(git_oid *source_oid,
        struct stat *source_stat,
        git_repository *repo,
        const String& source_path,
        const String& relative_path,
        IsIncludeOperator &IsIncluded)
{
    int rc;

    git_oid child_oid;
    struct stat source_stat_buf;

    if (!IsIncluded(relative_path)) {
        return 0;
    }

    if (source_stat == NULL) {
        source_stat = &source_stat_buf;
    }

    stat(source_path.c_str(), source_stat);

    if (S_ISREG(source_stat->st_mode)) {

        // Create a blob for the file
        rc = git_blob_create_fromdisk(source_oid, repo, source_path.c_str());
        if (rc < 0) {
            LOG("Cannot create a blob for file: %s", source_path.c_str());
            return -1;
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

        // Create a tree builder for the directory
        git_treebuilder *tree_builder;
        git_treebuilder_create(&tree_builder, NULL);

        while ((directory_entry = readdir(directory)) != NULL) {
            // Skip "." and ".."
            //DBG("d_name = %s", directory_entry->d_name);
            if ((!strcmp(directory_entry->d_name, ".")) ||
                    (!strcmp(directory_entry->d_name, "..")))
                continue;

            // Create an object for every child
            String child_absolute_path = source_path + '/' + directory_entry->d_name;
            String child_relative_path = relative_path + '/' + directory_entry->d_name;
            struct stat child_stat;

            rc = CreateObjectRecursive(&child_oid, &child_stat, repo,
                    child_absolute_path.c_str(), child_relative_path.c_str(), IsIncluded);
            if (rc < 0) {
                DBG("Failure in recursively create the object!");
                return rc;
            }

            // Set up tree entry name
            GitTreeEntryName tree_entry_name;
            tree_entry_name.Init(directory_entry->d_name, &child_stat);
            git_filemode_t tree_entry_mode = GitFileMode(tree_entry_name.mode);

            // Add the child into the tree builder
            rc = git_treebuilder_insert(NULL, tree_builder,
                    tree_entry_name.ToString().c_str(), &child_oid, tree_entry_mode);
            if (rc < 0) {
                LOG("Cannot insert object into tree builder for file: %s, errno = %d", child_absolute_path.c_str(), rc);
                DBG("tree entry name: %s", tree_entry_name.ToString().c_str());
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

// Checkout a commit
int GitVCS::Checkout(const String& repo_pathname,
        const String& commit_id,
        const String& relative_path,
        const String& destination_path)
{
    int rc;
    git_repository *repo;
    git_oid commit_oid;
    git_commit *commit;
    git_tree *commit_tree;
    git_oid relative_id;

    // Step 1: Open repository
    //
    rc = git_repository_open(&repo, repo_pathname.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot open repository.");
        return rc;
    }

    // Step 2: Retrieve the commit object
    //
    rc = git_oid_fromstr(&commit_oid, commit_id.c_str());
    if (rc < 0) {
        LOG("Failure: Commit id cannot be found.");
        return rc;
    }

    rc = git_commit_lookup(&commit, repo, &commit_oid);
    if (rc < 0) {
        LOG("Failure: Commit id cannot be found.");
        return rc;
    }

    // Step 3: Retrieve the commit tree
    //
    rc = git_commit_tree(&commit_tree, commit);
    if (rc < 0) {
        LOG("Failure: Commit id cannot be found.");
        return rc;
    }

    // Step 4: Find the root at the relative path
    //

    rc = GitTreeSearch(&relative_id, NULL, repo, commit_tree, relative_path);

    if (rc < 0) {
        LOG("Failure: Path %s does not exist.", relative_path.c_str());
        return rc;
    }

#if 0
    if (relative_id == NULL) {
        LOG("Failure: Cannot find path %s.", relative_path.c_str());
        return rc;
    }
#endif

    DBG("Relative root is");
    PrintOid(&relative_id);

    // Step 5: Overwrite files in destination
    rc = GitObjectWrite(repo, &relative_id, destination_path);
    if (rc < 0) {
        LOG("Failure: Cannot write into %s", destination_path.c_str());
        return rc;
    }

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
int GitVCS::PartialCommit(const String& repo_pathname,
        const String& old_commit_id,
        const String& relative_path,
        const String& work_dir,
        IsIncludeOperator &IsIncluded)
{
    int rc;
    git_repository *repo;
    git_tree *old_commit_tree, *new_commit_tree;
    git_oid root_oid, partial_oid, old_commit_oid, new_commit_id;
    git_commit *old_commit;
    git_signature *author, *committer;

    // Step 1: Open repository
    rc = git_repository_open(&repo, repo_pathname.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot open repository.");
        return rc;
    }

    // Step 2: Create partial tree
    rc = CreateObjectRecursive(&partial_oid, NULL, repo, work_dir, relative_path, IsIncluded);
    if (rc < 0) {
        LOG("Failed to create the partial tree!");
        return rc;
    }

#if 0
    // DEBUG CODE
    git_tree *partial_tree;
    rc = git_tree_lookup(&partial_tree, repo, &partial_oid);
    if (rc < 0) {
        DBG("partial_oid is not a tree!");
    } else {
        DBG("partial_oid is a tree");
        PrintTree(partial_tree);
    }
    // DEBUG CODE ENDS
#endif

    DBG("Partial id is:");
    PrintOid(&partial_oid);

    // Step 3: Create commit tree
    rc = git_oid_fromstr(&old_commit_oid, old_commit_id.c_str());
    if (rc < 0) {
        LOG("Failure: Commit id cannot be found.");
        return rc;
    }

    rc = git_commit_lookup(&old_commit, repo, &old_commit_oid);
    if (rc < 0) {
        LOG("Failure: Commit id cannot be found.");
        return rc;
    }

    rc = git_commit_tree(&old_commit_tree, old_commit);
    if (rc < 0) {
        LOG("Failure: Commit id cannot be found.");
        return rc;
    }

    DBG("Old commit tree is:");
    PrintTree(old_commit_tree);

    rc = CombineObject(&new_commit_tree, repo, old_commit_tree, &partial_oid, relative_path);
    if (rc < 0) {
        LOG("Failed to combine old commit with the new one!");
        return rc;
    }

    DBG("New commit tree is:");
    PrintTree(new_commit_tree);

    // Step 4: Write commit tree into HEAD
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

    git_commit_create_v(
            &new_commit_id,
            repo,
            "HEAD",
            author,
            committer,
            NULL,
            "",
            new_commit_tree,
            1,
            old_commit);

    // Step 5 Free resource
    git_signature_free(author);
    git_signature_free(committer);
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
    git_oid head_target_oid;
    char head_name[GIT_OID_HEXSZ+1];
    char *head_name_out;
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

    switch (git_reference_type(head_ref)) {
        case GIT_REF_OID:
            head_oid = git_reference_oid(head_ref);
            head_name_out = git_oid_tostr(head_name, GIT_OID_HEXSZ+1, head_oid);
            break;
        case GIT_REF_SYMBOLIC:
            head_target_name = git_reference_target(head_ref);
            rc = git_reference_name_to_oid(&head_target_oid, repo, head_target_name);
            if (rc < 0) {
                LOG("Failure: Cannot retrieve the oid of the HEAD.");
                DBG("repo_pathname=%s; rc=%d", repo_pathname.c_str(), rc);
                git_strarray ref_list;
                git_reference_list(&ref_list, repo, GIT_REF_LISTALL);
                for (int i = 0; i < ref_list.count; ++i) {
                    const char *refname = ref_list.strings[i];
                    DBG("refname=%s", refname);
                }
            }
            return rc;
            break;
        default:
            LOG("Unsupported git reference type");
    }

    if (rc < 0) {
        LOG("Failure: Cannot retrieve the oid of the HEAD.");
    }


    head_out = head_name_out;

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

    rc = git_reference_name_to_oid(&head_oid, repo, branch_name.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot retrieve the oid of the HEAD.");
        DBG("repo_pathname=%s; branch_name=%s, rc=%d", repo_pathname.c_str(), branch_name.c_str(), rc);
        git_strarray ref_list;
        git_reference_list(&ref_list, repo, GIT_REF_LISTALL);
        for (int i = 0; i < ref_list.count; ++i) {
            const char *refname = ref_list.strings[i];
            DBG("refname=%s", refname);
        }
        return rc;
    }

    head_name_out = git_oid_tostr(head_name, GIT_OID_HEXSZ+1, &head_oid);

    head_out = head_name_out;

    return 0;
}
