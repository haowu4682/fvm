// The implementation for git api as VCS
//

#include <fstream>
#include <git2.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

#include <common/common.h>
#include <common/util.h>
#include <vcs/gitvcs.h>

#define GIT_BINARY "/usr/bin/git"

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

// Commit the change into a specific repository
// Return 0 if succeeded, -1 otherwise.
int GitVCS::Commit(const String& repo_pathname,
        const Vector<String>& change_list)
{
    // TODO Implement
    return 0;
}

int GitTreeSearch(git_object **relative_object,
            git_repository *repo,
            git_tree *root_tree,
            const String& relative_path)
{
    int rc;
    Vector<String> relative_path_splitted;
    split(relative_path, "/", relative_path_splitted);

    git_tree *current_tree = root_tree;
    const git_tree_entry *tree_entry;
    git_object *current_object;

    for (int i = 0; i < relative_path_splitted.size(); ++i) {
        // Step 1: Find the tree entry
        tree_entry = git_tree_entry_byname(current_tree, relative_path_splitted[i].c_str());
        if (tree_entry == NULL) {
            LOG("path name not found: %s", relative_path_splitted[i].c_str());
            return -1;
        }

        // Step 2: Find the object
        git_tree_entry_to_object(&current_object, repo, tree_entry);
        if (rc < 0) {
            LOG("Unknown error while converting tree_entry to git_object");
            return rc;
        }

        // Step 3: Check if the object is tree object, and modify current_tree
        if (i != relative_path_splitted.size() - 1) {
            git_otype current_object_type = git_object_type(current_object);
            if (current_object_type != GIT_OBJ_TREE) {
                LOG("path name is not a directory: %s", relative_path_splitted[i].c_str());
                return -1;
            }

            const git_oid *current_object_oid = git_object_id(current_object);

            rc = git_tree_lookup(&current_tree, repo, current_object_oid);
            if (rc < 0) {
                LOG("Unknown error while converting git_object to git_tree");
            }
        }
    }

    return 0;
}

int GitBlobWrite(git_repository *repo, git_blob *blob, const String& destination)
{
    const void *buf = git_blob_rawcontent(blob);
    size_t size = git_blob_rawsize(blob);

    std::ofstream fout(destination.c_str());
    fout.write((char *)buf, size);
    fout.close();

    return 0;
}

struct GitTreeWriteCallbackPayload {
    git_repository *repo;
    const String *destination;
};

int GitTreeWriteCallback(const char *root,
        git_tree_entry *entry,
        void *payload)
{
    int rc;
    GitTreeWriteCallbackPayload *data =
        static_cast<GitTreeWriteCallbackPayload *> (payload);
    std::ostringstream file_name_stream;
    file_name_stream << *data->destination << "/" << root;
    file_name_stream << "/" << git_tree_entry_name(entry);
    String file_name = file_name_stream.str();

    git_otype entry_type = git_tree_entry_type(entry);
    const git_oid *entry_oid;
    git_blob *entry_blob;

    switch (entry_type) {
        case GIT_OBJ_TREE:
            // XXX: Use actual file mode!
            mkdir(file_name.c_str(), 0755);
            break;

        case GIT_OBJ_BLOB:
            entry_oid = git_tree_entry_id(entry);
            rc = git_blob_lookup(&entry_blob, data->repo, entry_oid);
            if (rc < 0) {
                LOG("Error in git data structure: blob is not found.");
            }
            GitBlobWrite(data->repo, entry_blob, file_name);
            break;

        default:
            LOG("Unknown type: %d", entry_type);
    }
}

int GitTreeWrite(git_repository *repo, git_tree *tree, const String& destination)
{
    int rc;

    struct GitTreeWriteCallbackPayload data;
    data.repo = repo;
    data.destination = &destination;
    rc = git_tree_walk(tree, GitTreeWriteCallback, GIT_TREEWALK_PRE, &data);
    if (rc < 0) {
        LOG("Cannot write the files into destination.");
        return rc;
    }

    return 0;
}

int GitObjectWrite(git_repository *repo, git_object *root, const String& destination)
{
    git_otype root_type = git_object_type(root);
    const git_oid *root_oid;
    git_blob *root_blob;
    git_tree *root_tree;
    int rc;

    switch (root_type) {
        case GIT_OBJ_TREE:
            break;

        case GIT_OBJ_BLOB:
            root_oid = git_object_id(root);
            rc = git_blob_lookup(&root_blob, repo, root_oid);
            if (rc < 0) {
                LOG("Error in git data structure: blob is not found.");
            }
            GitBlobWrite(repo, root_blob, destination);
            break;

        default:
            LOG("Unexpected type %d, skipped.", root_type);
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
    git_object *relative_root;

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
    rc = git_commit_tree(&commit_tree, commit);
    if (rc < 0) {
        LOG("Failure: Commit id cannot be found.");
        return rc;
    }

    rc = GitTreeSearch(&relative_root, repo,
            commit_tree, relative_path);
    if (rc < 0) {
        LOG("Failure: Path %s does not exist.", relative_path.c_str());
        return rc;
    }

    // Step 5: Overwrite files in destination
    rc = GitObjectWrite(repo, relative_root, destination_path);
    if (rc < 0) {
        LOG("Failure: Cannot write into %s", destination_path.c_str());
        return rc;
    }

    return 0;
}

