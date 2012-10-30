// The FVM Daemon

#include <limits.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include <common/common.h>
#include <common/util.h>
#include <vcs/gitvcs.h>
#include <vcs/vcs.h>

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

    // XXX Pass the vcs as a parameter
    VersionControlSystem *vcs = new GitVCS;

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
        // Format: COMMIT repo old_commit_id relative_path source_path
        if (command_args.size() < 5) {
            LOG("Format error for CHECKOUT command.");
        } else {
            vcs->PartialCommit(command_args[1], command_args[2], command_args[3],
                    command_args[4]);
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

    delete vcs;
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

    ListenAndResponse(port);

    return 0;
}

