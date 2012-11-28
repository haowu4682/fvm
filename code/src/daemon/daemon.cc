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

#include <common/common.h>
#include <common/util.h>
#include <config/branchmanager.h>
#include <config/config.h>
#include <vcs/gitvcs.h>
#include <vcs/vcs.h>

GitVCS *vcs;

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

void ResponseForClient(int client_sockfd)
{
    size_t size;
    char buffer[PATH_MAX + 100];

    // Read in the command from the client
    size = read(client_sockfd, buffer, PATH_MAX + 100);

    // Parse the command
    String buffer_str(buffer, size);
    Vector<String> command_args;

    LOG("Recieve Command: %s", buffer_str.c_str());

    split(buffer_str, " ", command_args);

    // Execute the command according to the command string
    if (command_args[0] == "CHECKOUT") {
        // Format: CHECKOUT repo commit_id relative_path destination_path
        if (command_args.size() < 5) {
            LOG("Format error for CHECKOUT command.");
        } else {
            vcs->Checkout(command_args[1], command_args[2], command_args[3],
                    command_args[4]);
        }
    } else if (command_args[0] == "COMMIT") {
        // Format: COMMIT repo branch_name relative_path source_path author email
        // num_of_tracelevel num_of_branch [path level]* [path branch]*
        if (command_args.size() < 9) {
            LOG("Format error for COMMIT command. Command args not enough. Given: %ld",
                    command_args.size());
        } else {
            String& repo_name = command_args[1];
            String& branch_name = command_args[2];
            String& relative_path = command_args[3];
            String& source_path = command_args[4];
            String& author_name = command_args[5];
            String& author_email = command_args[6];
            int trace_level_size = atoi(command_args[7].c_str());
            int branch_manager_size = atoi(command_args[8].c_str());

            size_t branch_start_index, trace_level_start_index = 9;

            int required_args_size = 9 + 2 * (trace_level_size + branch_manager_size);
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

            vcs->PartialCommit(repo_name, branch_name, relative_path, source_path,
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

int ListenAndResponse(int port)
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

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: fvm-daemon port\n");
        return 0;
    }

    int port = atoi(argv[1]);

    vcs = new GitVCS;

    ListenAndResponse(port);

    delete vcs;

    return 0;
}

