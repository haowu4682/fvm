// The FVM Daemon

#include <cstdlib>
#include <cstring>
#include <limits.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include <libssh/libssh.h>
#include <libssh/server.h>

#include <common/common.h>
#include <common/util.h>
#include <config/branchmanager.h>
#include <config/config.h>
#include <daemon/daemon.h>
#include <vcs/gitvcs.h>
#include <vcs/vcs.h>

#ifndef KEYS_FOLDER
#define KEYS_FOLDER "/etc/ssh/"
#endif // KEYS_FOLDER

void Daemon::ReadPublicKeyMap(std::istream& in) {
    String username, key_string;

    Vector<String> key_map_str_array;
    readline(in, key_map_str_array);

    for (int i = 0; i != key_map_str_array.size(); ++i) {
        String& str = key_map_str_array[i];
        int end_index = str.find_first_of(' ');

        username = str.substr(0, end_index);
        key_string = str.substr(end_index + 1/*, npos*/);

        ssh_key key = ssh_key_new();
        ssh_pki_import_pubkey_base64(key_string.c_str(), SSH_KEYTYPE_RSA, &key);

        public_key_map_[username] = key;
    }
}

void Daemon::WritePublicKeyMap(std::ostream& out) const {
    // XXX Magic Number
    char *buf = new char[10000];
    for (Map<String, ssh_key>::const_iterator it = public_key_map_.begin();
            it != public_key_map_.end(); ++it) {
        ssh_pki_export_pubkey_base64(it->second, &buf);

        out << it->first << ' ' << buf << std::endl;
    }
    delete buf;
}

#if 0
//// These are old design code, we keep them here in case we will use them
//// later.
class AlwaysTrueAdv : public IsIncludeOperator {
    virtual bool operator() (const String&) {
        return true;
    }
};

size_t ParseTraceLevel(const Vector<String>& source,
        size_t begin_index, int num,
        TraceLevelManager &manager)
{
    int count, index;
    index = begin_index;

    for (count = 0; count < num; ++count) {
        String pathname = source[index];
        String levelname = source[index+1];

        TraceLevel level = (TraceLevel) atoi(levelname.c_str());
        manager.AddTraceLevelItem(pathname, level);

        index += 2;
    }

    return index;
}

class TraceLevelManagerAdv : public TraceLevelManager, public IsIncludeOperator {
    public:
        virtual bool operator() (const String& pathname);
};

bool TraceLevelManagerAdv::operator() (const String& pathname)
{
    return GetTraceLevel(pathname) != kNone;
}

size_t ParseBranchManager(const Vector<String>& source,
        size_t begin_index, int num,
        BranchManager &manager)
{
    int count, index;
    index = begin_index;

    for (count = 0; count < num; ++count) {
        String pathname = source[index];
        String branch_name = source[index+1];

        manager.Add(pathname, branch_name);

        index += 2;
    }

    return index;
}

class BranchManagerAdv : public BranchManager, public BranchOperator {
    public:
        virtual String operator() (const String& pathname);
};

String BranchManagerAdv::operator() (const String& pathname)
{
    return GetBranch(pathname);
}
#endif

String Daemon::ResponseForPush(const Vector<String>& command_args,
        ssh_channel channel)
{
    LOG("Unimplemented command: %s", command_args[0].c_str());

    int rc;
    String ret_str;
    char *buf;

    // Step 1: Read in the pack
    int size;
    rc = ssh_channel_read(channel, &size, sizeof(int), false);
    if (rc < 0) {
        LOG("Unexpected size %d, ret = %d", size, rc);
        goto error;
    }

    // TODO Restrict package size not to be too large
    buf = new char[size];

    rc = ssh_channel_read(channel, buf, sizeof(buf), false);
    if (rc < size) {
        LOG("Failed to read in the entire package! read %d bytes, required %d bytes",
                rc, size);
        goto error2;
    }

    // Step 2: Call the verifier to verify the pack
    if (verifier == NULL) {
        LOG("No verifier found!");
        goto error2;
    } else {
        ret_str = verifier->VerifyPackage(buf, size);
    }

    // Step 3: Return the response
    return ret_str;

error2:
    delete buf;
error:
    return "";
}

void Daemon::ResponseForClient(ssh_session session, ssh_channel channel)
{
    int rc;
    size_t size;
    char buffer[PATH_MAX + 100];

    // Read in the command from the client
    size = ssh_channel_read(channel, buffer, PATH_MAX + 100, false);

    // Parse the command
    String buffer_str(buffer, size);
    Vector<String> command_args;

    LOG("Recieve Command: %s", buffer_str.c_str());

    split(buffer_str, " ", command_args);

    // Execute the command according to the command string
    if (command_args[0] == "PUSH") {
        String ret_str = ResponseForPush(command_args, channel);

        size = ssh_channel_write(channel, ret_str.c_str(), ret_str.size());
        if (size != ret_str.size()) {
            LOG("Failed to send return message: %s", ret_str.c_str());
        }
    } else {
        // Undefined command, do nothing
        LOG("Undefined command: %s", command_args[0].c_str());
    }

    // Finalize the channel
    ssh_channel_close(channel);
    ssh_disconnect(session);
}

#if 0
void Daemon::ResponseForClient(int sockfd)
{
    //// These are old design code, we keep them here in case we will use them
    //// later.
    size_t size;
    char buffer[PATH_MAX + 100];

    // Read in the command from the client
    size = read(client_sockfd, buffer, PATH_MAX + 100);

    // Parse the command
    String buffer_str(buffer, size);
    Vector<String> command_args;

    LOG("Recieve Command: %s", buffer_str.c_str());

    split(buffer_str, " ", command_args);

    if (command_args[0] == "CHECKOUT") {
        // Format: CHECKOUT repo commit_id relative_path destination_path username
        if (command_args.size() < 6) {
            LOG("Format error for CHECKOUT command.");
        } else {
            vcs->Checkout(command_args[1], command_args[2], command_args[3],
                    command_args[4], command_args[5]);
        }
    } else if (command_args[0] == "COMMIT") {
        // Format: COMMIT repo branch_name relative_path source_path author email username
        // num_of_tracelevel num_of_branch [path level]* [path branch]*
        if (command_args.size() < 10) {
            LOG("Format error for COMMIT command. Command args not enough. Given: %ld",
                    command_args.size());
        } else {
            String& repo_name = command_args[1];
            String& branch_name = command_args[2];
            String& relative_path = command_args[3];
            String& source_path = command_args[4];
            String& author_name = command_args[5];
            String& author_email = command_args[6];
            String& username = command_args[7];
            int trace_level_size = atoi(command_args[8].c_str());
            int branch_manager_size = atoi(command_args[9].c_str());

            size_t branch_start_index, trace_level_start_index = 10;

            int required_args_size = 10 + 2 * (trace_level_size + branch_manager_size);
            if (command_args.size() < required_args_size) {
                LOG("Command args not enough! Required:%d, Given: %ld",
                        required_args_size, command_args.size());
                return;
            }

            TraceLevelManagerAdv trace_level_manager;
            BranchManagerAdv branch_manager;
            branch_start_index = ParseTraceLevel(command_args,
                    trace_level_start_index, trace_level_size, trace_level_manager);
            ParseBranchManager(command_args, branch_start_index, branch_manager_size, branch_manager);

            DBG("Trace level manager list: %s", trace_level_manager.ToString().c_str());
            DBG("Branch manager list: %s", branch_manager.ToString().c_str());

            vcs->username(author_name);
            vcs->user_email(author_email);

            vcs->PartialCommit(repo_name, branch_name, relative_path, source_path, username,
                    trace_level_manager, branch_manager);
        }
    } else if (command_args[0] == "GETHEAD") {
        // Format GETHEAD repo [branch]
        String head_commit_str;
        if (command_args.size() < 2) {
            LOG("Format GETHEAD repo [branch]");
        } else if (command_args.size() == 2) {
            vcs->GetHead(command_args[1], head_commit_str);
        } else {
            vcs->GetHead(command_args[1], command_args[2], head_commit_str);
        }

        LOG("head commit: %s", head_commit_str.c_str());
        write(client_sockfd, head_commit_str.c_str(), head_commit_str.size());
    } else if (command_args[0] == "BRANCHCREATE") {
        vcs->BranchCreate(command_args[1], command_args[2], command_args[3]);
    } else {
        // Undefined command, do nothing
        LOG("Undefined command: %s", command_args[0].c_str());
    }
}
#endif

bool Daemon::CheckAuthentication(ssh_message message)
{
    const char* username = ssh_message_auth_user(message);
    ssh_key message_key = ssh_message_auth_pubkey(message);

    ssh_key database_key = public_key_map_[username];

    if (ssh_key_cmp(message_key, database_key, SSH_KEY_CMP_PUBLIC)) {
        return false;
    } else {
        return true;
    }
}

int Daemon::ListenAndResponse()
{
    int rc;
    ssh_bind sshbind;
    ssh_session session;
    ssh_channel channel;
    ssh_message message;
    bool auth;

    // Initialize libssh structures
    sshbind = ssh_bind_new();

    // Set up default options
    rc = ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT, &port);
    if (rc < 0) {
        LOG("Cannot Set the port to listen! Daemon start fails.");
        return rc;
    }

    rc = ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY,
            KEYS_FOLDER "ssh_host_rsa_key");
    if (rc < 0) {
        LOG("Cannot Set the host rsa key! Daemon start fails.");
        return rc;
    }

    rc = ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY,
            KEYS_FOLDER "ssh_host_dsa_key");
    if (rc < 0) {
        LOG("Cannot Set the host dsa key! Daemon start fails.");
        return rc;
    }

    // Listen and Accept
    rc = ssh_bind_listen(sshbind);
    if (rc < 0) {
        LOG("Error listening to socket: %s", ssh_get_error(sshbind));
        return rc;
    }

    for (;;) {
        session = ssh_new();
        auth = false;
        channel = 0;

        rc = ssh_bind_accept(sshbind, session);
        if (rc < 0) {
            LOG("Error accepting an incoming session: %s", ssh_get_error(sshbind));
            // XXX Is "return rc" a better way here and in following similar
            // context?
            continue;
        }

        rc = ssh_handle_key_exchange(session);
        if (rc < 0) {
            LOG("Error exchange of key: %s", ssh_get_error(session));
            continue;
        }

        // Authentication
        do {
            message = ssh_message_get(session);
            if (!message) {
                break;
            }

            switch (ssh_message_type(message)) {
                case SSH_REQUEST_AUTH:
                    switch (ssh_message_subtype(message)) {
                        // Since public key is required in the system, we also
                        // require the user to use public key authentication here.
                        case SSH_AUTH_METHOD_PUBLICKEY:
                            // check public key authentication here.
                            if (CheckAuthentication(message)) {
                                auth = true;
                                ssh_message_auth_reply_success(message, 0);
                                break;
                            }

                            // If the user is trying to using a
                            // wrong key, we just let the control flow going
                            // through to reply rejection, thus we do not
                            // insert "break" sentence here.

                        default:
                            // If the user is not using public key, we need to
                            // tell the user to authenticate with PUBLICKEY
                            // method.
                            ssh_message_auth_set_methods(message,
                                    SSH_AUTH_METHOD_PUBLICKEY);
                            ssh_message_reply_default(message);
                            break;
                    }

                    break;

                default:
                    // For non-authentication messge at the time,
                    // reply with a default REJECTION to tell the user to
                    // authenticate.
                    ssh_message_reply_default(message);
                    break;
            }

            ssh_message_free(message);
        } while (!auth);

        if (!auth) {
            LOG("Authentication fails. Connection closed. Error msg: %s",
                    ssh_get_error(session));
            ssh_disconnect(session);
            continue;
        }

        do {
            message = ssh_message_get(session);
            if (!message) {
                break;
            }

            switch (ssh_message_type(message)) {
                case SSH_REQUEST_CHANNEL_OPEN:
                    if (ssh_message_subtype(message) == SSH_CHANNEL_SESSION) {
                        channel = ssh_message_channel_request_open_reply_accept(message);
                        break;
                    }

                default:
                    // For a message other than channel open, we reject the
                    // mesage.
                    ssh_message_reply_default(message);
            }

            ssh_message_free(message);

        } while(!channel);

        if (!channel) {
            LOG("Cannot open SSH channel! Error msg: %s", ssh_get_error(session));
            ssh_disconnect(session);
            continue;
        }

        // The child thread is responsible to free the session after it is used.
        ResponseForClient(session, channel);
    }

    ssh_bind_free(sshbind);

    return 0;
}

#if 0
//// The following code is obsoleted, it does not use SSH protocol.
//// We may get it back in the future so we keep it right here.
int Daemon::ListenAndResponse()
{
    int sockfd, client_sockfd;
    socklen_t client_address_len;
    char buffer[256];
    struct sockaddr_in server_address, client_address;
    int n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOG("ERROR opening socket");
        return -1;
    }

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &server_address,
                sizeof(server_address)) < 0) {
        LOG("ERROR on binding");
        return -1;
    }

    listen(sockfd,5);
    client_address_len = sizeof(client_address);

    for (;;) {
        client_sockfd = accept(sockfd,
                (struct sockaddr *) &client_address,
                &client_address_len);
        if (client_sockfd < 0)
            LOG("ERROR on accept");
        printf("Connection established.\n");
        ResponseForClient(client_sockfd);
        close(client_sockfd);
    }

    close(sockfd);
    return 0;
}
#endif

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: fvm-daemon port\n");
        return 0;
    }

    EncryptionManager *encrypt_manager = new StandardEncryptionManager;
    // TODO Use actual user name
    KeyManager *key_manager = new KeyManager("haowu");
    AccessManager *access_manager = new AccessManager(key_manager);
    Verifier verifier(access_manager);

    Daemon daemon(atoi(argv[1]), &verifier);

    daemon.ListenAndResponse();

    delete encrypt_manager;
    delete key_manager;
    delete access_manager;

    return 0;
}

