
#ifndef DAEMON_DAEMON_H__
#define DAEMON_DAEMON_H__

#include <sstream>
#include <map>

#include <libssh/libssh.h>

#include <common/common.h>

class Daemon {
    public:
        Daemon(int port) : port(port) {}
//            vcs = new GitVCS;
//        }

        ~Daemon() {}
//            delete vcs;
//        }
//

#if 0
        void ReadPublicKeyMap(std::istream &in);
        void WritePublicKeyMap(std::ostream &out) const;
#endif

        int ListenAndResponse();

    protected:
        void ResponseForClient(ssh_session session, ssh_channel channel);
        int ResponseForPush();

    private:
        int port;
//        GitVCS *vcs;

        // The public key list
        //Map<String, String> public_key_map_;
};

#if 0
istream& operator >> (istream& in, Daemon& daemon) {
    daemon.ReadPublicKeyMapFromStream(in);
}
#endif

#endif // DAEMON_DAEMON_H__

