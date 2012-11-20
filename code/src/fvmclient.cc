// The implementation of FVM client

#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <common/common.h>
#include <fvmclient.h>

int FVMClient::Connect()
{
    int n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        LOG("ERROR opening socket");
        return -1;
    }

    server = gethostbyname(hostname_.c_str());
    if (server == NULL) {
        LOG("ERROR, no such host\n");
        return -1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(port_);

    if (connect(sockfd_,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        LOG("ERROR connecting");
    }

    connected_ = true;

    return 0;
}

void FVMClient::Disconnect() {
    connected_ = false;

    close(sockfd_);
}

// Make a partial checkout call to the daemon
int FVMClient::Checkout(const String &repo_path, const String &link_src,
        const String &link_dst, const String &commit_id)
{
    std::ostringstream command_sout;
    command_sout << "CHECKOUT " << repo_path << ' '
                 << commit_id << ' '
                 << link_src << ' '
                 << link_dst;
    String command_line = command_sout.str();

    int rc = write(sockfd_, command_line.c_str(), command_line.size());
    if (rc < 0) {
        LOG("Failed to send command to the server: %s", command_line.c_str());
    }

    return 0;
}

// Make a partial commit call to the daemon
int FVMClient::Commit(const String &repo_path, const String &link_src,
        const String &link_dst, const String &branch_name,
        RepoConfig *config /* = NULL */, BranchManager *branch_manager /* = NULL */)
{
    std::ostringstream command_sout;
    const Map<String, TraceLevel>* trace_level_map;
    const Map<String, String>* branch_map;

    command_sout << "COMMIT " << repo_path << ' '
                 << branch_name << ' '
                 << link_src << ' '
                 << link_dst << ' '
                 << username_ << ' '
                 << user_email_;
    if (config != NULL) {
        trace_level_map = config->trace_level_map();
    }

    if (branch_manager != NULL) {
        branch_map = &branch_manager->branch_map();
    }

    if (config != NULL) {
        for (Map<String, TraceLevel>::const_iterator it = trace_level_map->begin();
                it != trace_level_map->end(); ++it) {
            command_sout << ' ' << it->first
                         << ' ' << it->second;
        }
    }

    if (branch_manager != NULL) {
        for (Map<String, String>::const_iterator it = branch_map->begin();
                it != branch_map->end(); ++it) {
            command_sout << ' ' << it->first
                         << ' ' << it->second;
        }
    }

    String command_line = command_sout.str();

    int rc = write(sockfd_, command_line.c_str(), command_line.size());
    if (rc < 0) {
        LOG("Failed to send command to the server: %s", command_line.c_str());
    }

    return 0;
}

// Retrieve the head of a repository
int FVMClient::RetrieveHead(const String &repo_path, String &head_out)
{
    // XXX: Magic number
    char buf[1096];
    std::ostringstream command_sout;
    command_sout << "GETHEAD " << repo_path;
    String command_line = command_sout.str();

    int rc = write(sockfd_, command_line.c_str(), command_line.size());
    if (rc < 0) {
        LOG("Failed to send command to the server: %s", command_line.c_str());
    }

    rc = read(sockfd_, buf, 1096);
    if (rc < 0) {
        LOG("Failed to read reply from the server for command: %s", command_line.c_str());
    }
    LOG("head commit = %s", buf);

    head_out = buf;

    return 0;
}

// Retrieve the head of a repository
int FVMClient::RetrieveHead(const String &repo_path, const String& branch_name, String &head_out)
{
    // XXX: Magic number
    char buf[1096];
    std::ostringstream command_sout;
    command_sout << "GETHEAD " << repo_path << " " << branch_name;
    String command_line = command_sout.str();

    int rc = write(sockfd_, command_line.c_str(), command_line.size());
    if (rc < 0) {
        LOG("Failed to send command to the server: %s", command_line.c_str());
    }

    rc = read(sockfd_, buf, 1096);
    if (rc < 0) {
        LOG("Failed to read reply from the server for command: %s", command_line.c_str());
    }
    LOG("head commit = %s", buf);

    head_out = buf;

    return 0;
}
