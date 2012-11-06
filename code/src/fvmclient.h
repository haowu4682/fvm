// The files defines a class FVMClient which talks to the FVM Daemon

#ifndef FVM_CLIENT_H_
#define FVM_CLIENT_H_

#include <string>

#include <common/common.h>
#include <single/config.h>

class FVMClient {
    public:
        // Constructor
        FVMClient(const String &username, const String &user_email) {
            username_ = username;
            user_email_ = user_email;
        }

        // Accessors and Mutators
        void hostname(const String& value) { hostname_ = value; }
        String hostname() { return hostname_; }
        void port(int value) { port_ = value; }
        int port() { return port_; }
        int sockfd() { return sockfd_; }
        bool connected() { return connected_; }

        // Connect to the server
        int Connect();

        // Disconnect from the server
        void Disconnect();

        // Make a partial checkout call to the daemon
        int Checkout(const String &repo_path, const String &link_src,
                const String &link_dst, const String &commit_id);

        // Make a partial commit call to the daemon
        int Commit(const String &repo_path, const String &link_src,
                const String &link_dst, const String &old_commit_id,
                RepoConfig *config = NULL);

        // Retrieve the head of a repository
        int RetrieveHead(const String &repo_path, String &head_out);

    private:
        // Basic server data
        String hostname_;
        int port_;

        // Connection data
        bool connected_;
        int sockfd_;

        // User data
        String username_;
        String user_email_;
};

#endif // FVM_CLIENT_H_

