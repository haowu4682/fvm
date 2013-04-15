#ifndef ENCRYPTION_MANAGER_H__
#define ENCRYPTION_MANAGER_H__

#include <string>

#include <common/common.h>

class EncryptionManager {
    public:
        virtual size_t Encrypt(char *output, const char *input, size_t size,
                const String &key, const String &ivec) = 0;

        virtual size_t Decrypt(char *output, const char *input, size_t size,
                const String &key, const String &ivec) = 0;

        virtual size_t GetIVSize() = 0;

        virtual size_t GetKeySize() = 0;

};

class StandardEncryptionManager : public EncryptionManager{
    public:
        virtual size_t Encrypt(char *output, const char *input, size_t size,
                const String &key, const String &ivec);

        virtual size_t Decrypt(char *output, const char *input, size_t size,
                const String &key, const String &ivec);

        virtual size_t GetIVSize();

        virtual size_t GetKeySize();

};

#endif // ENCRYPTION_MANAGER_H__

