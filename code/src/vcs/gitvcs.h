// The file describes the interface for a Version Control System (VCS)

#ifndef VCS_GITVCS_H_
#define VCS_GITVCS_H_

#include <git2.h>
#include <string>
#include <sys/stat.h>
#include <vector>

#include <libssh/libssh.h>

#include <common/accessinfo.h>
#include <common/common.h>
#include <vcs/access.h>
#include <vcs/vcs.h>
#include <vcs/verifier.h>

struct GitTreeEntryName {
    mode_t mode;
    String user;
    String group;
    String name;

    String ToString() const;
    int FromString(const String& str);
    int Init(const String& name, const struct stat* stat);
    int WriteToFile(const String& filepath) const;

    GitTreeEntryName();
    GitTreeEntryName(const String& str);
    GitTreeEntryName(const String& name, const struct stat* stat);
};

class FVMTransport : public git_transport {
    public:
        FVMTransport();

        int Connect(const char *url,
                git_cred_acquire_cb cred_acquire_cb,
                void *cred_acquire_payload,
                int direction,
                int flags);

        int IsConnected();

        int Push(git_push *push);

        int Close();

        static void Free(struct git_transport *transport);

    private:
        bool is_connected_;

        int port;

        ssh_session session;
        ssh_channel channel;
};

// The Interface for a version control system
class GitVCS : public VersionControlSystem {
    public:
        // Constructors
        GitVCS() : access_list_filename_(".fvmaccesslist") {}
        virtual ~GitVCS() {}

        // Mutators
        void username(const String &value) { username_ = value; }
        void user_email(const String &value) { user_email_ = value; }
        String username() { return username_; }
        String user_email() { return user_email_; }

        void verifier(Verifier *value) { verifier_ = value;}
        Verifier* verifier() { return verifier_; }
        void access_manager(AccessManager *value) { access_manager_ = value;}
        AccessManager* access_manager() { return access_manager_; }
        void encryption_manager(EncryptionManager *value) { encryption_manager_ = value;}
        EncryptionManager* encryption_manager() { return encryption_manager_; }

        // Get the change list for a specific repository
        // Return the number of items to be included in the change list.
        virtual int GetChangeList(const String& repo_pathname,
                Vector<String>& change_list);

        // Commit the change into a specific repository
        // Return 0 if succeeded, -1 otherwise.
        virtual int Commit(const String& repo_pathname,
                const Vector<String>& change_list);

        // Checkout a commit
        virtual int Checkout(const String& repo_pathname,
                const String& commit_id,
                const String& relative_path,
                const String& destination_path,
                const String& username);

        // Partial commit
        virtual int PartialCommit(const String& repo_pathname,
                const String& branch_name,
                const String& relative_path,
                const String& work_dir,
                const String& username,
                IsIncludeOperator &IsIncluded,
                BranchOperator &GetBranch);

        // Retrieve the HEAD commit
        virtual int GetHead(const String& repo_pathname,
                const String& branch_name,
                String& head_out);
        virtual int GetHead(const String& repo_pathname,
                String& head_out);

        // Create a branch
        virtual int BranchCreate(const String& repo_name,
                const String& branch_name,
                const String& head_commit_id);

        // Create a push object, and push it to remote side
        virtual int SendPush(const String& repo_pathname, const String& remote_name);

        // Recieve a push object, and write the objects into the repository
        static int ReceivePush(git_push* push);

    private:
        String username_;
        String user_email_;
        String access_list_filename_;

        // Encryption stuff
        // TODO Initilize them
        EncryptionManager *encryption_manager_;
        Verifier *verifier_;
        AccessManager *access_manager_;

        // Auxilary functions
        int ReadAccessList(
                // Output
                AccessList& access_list,
                // Input
                git_repository* repo,
                git_tree* root_tree);

        int WriteAccessList(
                // Output
                git_tree** new_root_tree,
                // Input
                git_repository* repo,
                const AccessList& access_list,
                git_tree* old_root_tree);

        int GitObjectWrite(git_repository *repo,
                const git_oid *root_oid,
                const String& destination,
                const String& username,
                AccessList &access_list);

        int GitBlobWrite(git_repository *repo, git_blob *blob,
                const String& destination, const GitTreeEntryName &name, const String& relative_path);

        int GitTreeWrite(git_repository *repo, git_tree *tree,
                const String& destination,
                const String& username, AccessList &list);

        friend int GitTreeWriteCallback(const char *root,
                const git_tree_entry *entry,
                void *payload);

        size_t GetEncryptedContent(char **file_encrypted_content_ptr,
                struct stat* source_stat,
                const String& source_path,
                const String& root_path);

        int CreateObjectRecursive(
                // Output
                git_oid *source_oid,
                struct stat *source_stat,
                AccessList *new_access_list,

                // Input
                git_repository *repo,
                const String& username,
                const String& branch_name,
                const String& root_path,
                const String& source_path,
                const String& relative_path,
                const git_oid *old_object_oid,
                //GitTreeEntryName *old_tree_entry_name,
                AccessList *old_access_list,

                // Operator
                IsIncludeOperator &IsIncluded,
                BranchOperator &GetBranch);
};

#endif // VCS_GITVCS_H_

