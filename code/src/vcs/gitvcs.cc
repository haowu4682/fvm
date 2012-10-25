// The implementation for git api as VCS
//

#include <dirent.h>
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

int GitTreeSearch(git_object **relative_object,
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
    git_object *current_object;

    for (int i = 0; i < relative_path_splitted.size(); ++i) {
        if (tree_link_path != NULL) {
            tree_link_path->push_back(current_tree);
        }
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
            mkdir(file_name.c_str(),  0100755);
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

int CombineObject(git_tree **new_root_tree,
        git_repository *repo,
        git_tree *root_tree,
        git_oid combine_object_oid,
        const String& combine_point)
{
    int rc;
    git_object *old_combine_object;
    Vector<git_tree *> tree_link_path;

    // Divide the combine point
    Vector<String> combine_point_list;
    split(combine_point, "/", combine_point_list);

    // Search for the combine point in the root tree
    rc = GitTreeSearch(&old_combine_object, &tree_link_path, repo, root_tree,
            combine_point);
    if (rc < 0) {
        LOG("The combine point %s does not exist.", combine_point.c_str());
        return rc;
    }

    // Recursively modify the tree, up till the root
    git_oid current_new_oid;
    git_tree *current_parent_tree;
    git_treebuilder *tree_builder;
    current_new_oid = combine_object_oid;

    if (combine_point_list.size() != tree_link_path.size()) {
        LOG("Unknown error in GitTreeSearch: combine_point_list_size = %ld, "
                "tree_link_path_size = %ld", combine_point_list.size(), tree_link_path.size());
        return -1;
    }

//    for (Vector<git_tree *>::reverse_iterator tree_link_path_ptr = tree_link_path.rbegin();
//            tree_link_path_ptr != tree_link_path.rend(); ++tree_link_path_ptr) {
    for (int i = combine_point_list.size() - 1; i >= 0; --i) {
        current_parent_tree = tree_link_path[i];
        String &current_child_name = combine_point_list[i];

        git_treebuilder_create(&tree_builder, current_parent_tree);

        // XXX Use actual mode
        git_treebuilder_insert(NULL, tree_builder, current_child_name.c_str(),
                &current_new_oid, 0644);

        git_treebuilder_free(tree_builder);
    }

    return 0;
}

int CreateObjectRecursive(git_oid *source_oid,
        mode_t *source_mode,
        git_repository *repo,
        const String& source_path)
{
    int rc;

    git_oid child_oid;
    struct stat source_stat;

    stat(source_path.c_str(), &source_stat);
    if (source_mode != NULL) {
        *source_mode = source_stat.st_mode;
    }

    if (S_ISREG(source_stat.st_mode)) {

        // Create a blob for the file
        rc = git_blob_create_fromdisk(source_oid, repo, source_path.c_str());
        if (rc < 0) {
            LOG("Cannot create a blob for file: %s", source_path.c_str());
            return -1;
        }

    } else if (S_ISDIR(source_stat.st_mode)) {
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
            // Create an object for every child
            String child_absolute_path = source_path + '/' + directory_entry->d_name;
            mode_t child_mode;

            rc = CreateObjectRecursive(&child_oid, &child_mode, repo,
                    child_absolute_path.c_str());
            if (rc < 0) {
                return rc;
            }

            // Add the child into the tree builder
            // XXX Do not add the unsupported files
            rc = git_treebuilder_insert(NULL, tree_builder,
                    directory_entry->d_name, &child_oid, child_mode);
            if (rc < 0) {
                LOG("Cannot insert object into tree builder for file: %s", child_absolute_path.c_str());
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

    rc = GitTreeSearch(&relative_root, NULL, repo,
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
//        const Vector<String>& change_list,
        const String& old_commit_id,
        const String& relative_path,
        const String& work_dir)
{
    int rc;
    git_repository *repo;
    git_tree *old_commit_tree, *new_commit_tree;
    git_oid root_oid, partial_oid, old_commit_oid;
    git_commit *old_commit;

    // Step 1: Open repository
    rc = git_repository_open(&repo, repo_pathname.c_str());
    if (rc < 0) {
        LOG("Failure: Cannot open repository.");
        return rc;
    }

    // Step 2: Create partial tree
    CreateObjectRecursive(&partial_oid, NULL, repo, work_dir);

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

    CombineObject(&new_commit_tree, repo, old_commit_tree, partial_oid, relative_path);

    return 0;
}

