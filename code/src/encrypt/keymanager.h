#ifndef KEY_MANAGER_H__
#define KEY_MANAGER_H__

#include <string>

#include <openssl/rsa.h>

#include <common/common.h>

class KeyManager {
    public:
        KeyManager (const String &username)
        {
            username_ = username;
            user_private_key_ = NULL;
            user_public_key_ = NULL;
        }

#if 0
        KeyManager (const String &username,
                const String &private_key,
                const String &public_key)
            : username(username), user_private_key(private_key),
            user_public_key(public_key) {}
#endif

        void ReadPublicKeyFromFile(const String &filename);

        void ReadPrivateKeyFromFile(const String &filename);

        virtual String GetGroupKeyFileName(const String &group_name);

        // Decryption of Group Key Value, using private key.
        virtual String GetGroupKeyContent(const String &file_content);

        // Encryption of Group Key Value, using public key.
        virtual String GetNewGroupKeyFileContent(const String &key_content);

    protected:
        String username_;

    private:
        RSA *user_private_key_;
        RSA *user_public_key_;
};


#endif // KEY_MANAGER_H__

