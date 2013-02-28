
#ifndef DAEMON_DAEMON_H__
#define DAEMON_DAEMON_H__

#include <sstream>
#include <map>

#include <libssh/libssh.h>

#include <common/common.h>
#include <daemon/verifier.h>

class Daemon {
    public:
        Daemon(int port, Verifier* verifier) : port(port), verifier(verifier) {}
//            vcs = new GitVCS;
//        }

        ~Daemon() {}
//            delete vcs;
//        }
//

        void ReadPublicKeyMap(std::istream &in);
        void WritePublicKeyMap(std::ostream &out) const;

        int ListenAndResponse();

    protected:
        void ResponseForClient(ssh_session session, ssh_channel channel);
        int ResponseForPush();
        bool CheckAuthentication(ssh_message message);

    private:
        int port;
//        GitVCS *vcs;

        // The public key list, which maps user name to public key
        Map<String, ssh_key> public_key_map_;

        // The verifier used to verify "push" request
        Verifier *verifier;
};

#if 0
istream& operator >> (istream& in, Daemon& daemon) {
    daemon.ReadPublicKeyMapFromStream(in);
}
#endif

#endif // DAEMON_DAEMON_H__

