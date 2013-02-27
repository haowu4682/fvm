#ifndef DAEMON_VERIFIER__
#define DAEMON_VERIFIER__

#include <git2.h>

#include <encrypt/encryptionmanager.h>
#include <encrypt/keymanager.h>

class Verifier {
    public:
        Verifier(KeyManager *key_manager,
                EncryptionManager *encryption_manager)
            : key_manager(key_manager),
              encryption_manager(encryption_manager) {}

        // Returns TRUE if the tree does not violate permission
        bool VerifyTree(git_tree *tree);

    private:
        KeyManager *key_manager;
        EncryptionManager *encryption_manager;
};

#endif // DAEMON_VERIFIER__

