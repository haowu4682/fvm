#ifndef ENCRYPTION_MANAGER_H__
#define ENCRYPTION_MANAGER_H__

#include <string>

#include <common/common.h>

class EncryptionManager {
    public:
        virtual void Encrypt(char *output, const char *input, size_t size,
                const String &key, const String &ivec) = 0;

        virtual void Decrypt(char *output, const char *input, size_t size,
                const String &key, const String &ivec) = 0;

};

class StandardEncryptionManager : public EncryptionManager{
    public:
        virtual void Encrypt(char *output, const char *input, size_t size,
                const String &key, const String &ivec);

        virtual void Decrypt(char *output, const char *input, size_t size,
                const String &key, const String &ivec);

};

#endif // ENCRYPTION_MANAGER_H__

