#ifndef VCS_VERIFIER__
#define VCS_VERIFIER__

#include <git2.h>

#include <encrypt/encryptionmanager.h>
#include <encrypt/keymanager.h>

class Verifier {
    public:
        Verifier(//KeyManager *key_manager,
                EncryptionManager *encryption_manager)
            : //key_manager(key_manager),
              encryption_manager(encryption_manager) {}

        // Verify the whole push package
        // buf: the incoming package
        // size: the incoming package size
        // ret: the response
        String VerifyPackage(const char* buf, size_t size);

    protected:
        // Returns TRUE if the tree does not violate permission
        bool VerifyTree(git_tree *tree);

        // Verify a reference update
        //bool VerifyReference(git_

        // Verify a push object
        void VerifyPush(git_push *push/*,  git_repository *repo */ );

    private:
//        KeyManager *key_manager;
        EncryptionManager *encryption_manager;
};

#endif // VCS_VERIFIER__

