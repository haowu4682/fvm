
#include <cstring>

#include <openssl/aes.h>

#include <encrypt/encryptionmanager.h>

void GetAESKey(AES_KEY &key_value, const String &key_str)
{
}

void StandardEncryptionManager::Encrypt(char *output, const char *input,
        size_t size, const String &key, const String &ivec)
{
    AES_KEY aes_key;
    GetAESKey(aes_key, key);

    unsigned char *ivec_buffer = new unsigned char[ivec.size()];
    strcpy((char *)ivec_buffer, ivec.c_str());

    AES_cbc_encrypt((unsigned char *)input, (unsigned char *)output, size,
            &aes_key, ivec_buffer, AES_ENCRYPT);

    delete[] ivec_buffer;
}

void StandardEncryptionManager::Decrypt(char *output, const char *input,
        size_t size, const String &key, const String &ivec)
{
    AES_KEY aes_key;
    GetAESKey(aes_key, key);

    unsigned char *ivec_buffer = new unsigned char[ivec.size()];
    strcpy((char *)ivec_buffer, ivec.c_str());

    AES_cbc_encrypt((unsigned char *)input, (unsigned char *)output, size,
            &aes_key, ivec_buffer, AES_DECRYPT);

    delete[] ivec_buffer;
}

size_t StandardEncryptionManager::GetKeySize()
{
    return AES_BLOCK_SIZE;
}

size_t StandardEncryptionManager::GetIVSize()
{
    return AES_BLOCK_SIZE;
}


