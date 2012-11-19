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
#include <single/config.h>
#include <vcs/gitvcs.h>
#include <vcs/vcs.h>

GitVCS *vcs;

class AlwaysTrueAdv : public IsIncludeOperator {
    virtual bool operator() (const String&) {
        return true;
    }
};

void ParseTraceLevel(const Vector<String>& source,
        size_t begin_index, size_t end_size,
        TraceLevelManager &manager)
{
    for (size_t i = begin_index; i < end_size - 1; i = i + 2) {
        String pathname = source[i];
        String levelname = source[i+1];
        //TraceLevel level = TraceLevelManager::GetTraceLevelFromString(levelname);
        TraceLevel level = (TraceLevel) atoi(levelname.c_str());
        manager.AddTraceLevelItem(pathname, level);
    }
}

// TODO Better naming
class TraceLevelManagerAdv : public TraceLevelManager, public IsIncludeOperator {
    public:
        virtual bool operator() (const String& pathname);
};

bool TraceLevelManagerAdv::operator() (const String& pathname)
{
    return GetTraceLevel(pathname) != kNone;
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
        // Format: COMMIT repo old_commit_id relative_path source_path author email
        if (command_args.size() < 7) {
            LOG("Format error for COMMIT command.");
        } else if (command_args.size() == 7) {
            vcs->username(command_args[5]);
            vcs->user_email(command_args[6]);
            AlwaysTrueAdv AlwaysTrue;
//            vcs->PartialCommit(command_args[1], command_args[2], command_args[3],
//                    command_args[4], AlwaysTrue);
        } else {
            TraceLevelManagerAdv manager;
            ParseTraceLevel(command_args, 7, command_args.size(), manager);
            vcs->username(command_args[5]);
            vcs->user_email(command_args[6]);
//            vcs->PartialCommit(command_args[1], command_args[2], command_args[3],
//                    command_args[4], manager);
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

