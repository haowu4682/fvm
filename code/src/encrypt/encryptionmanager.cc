
#include <cstring>

#include <openssl/aes.h>

#include <encrypt/encryptionmanager.h>

void GetAESKey(AES_KEY &key_value, const String &key_str, const int enc)
{
    int rc;
    if (enc) {
        rc = AES_set_encrypt_key((const unsigned char *)key_str.c_str(),
                /*key_str.size() * sizeof(char)*/AES_KEY_SIZE, &key_value);
    } else {
        rc = AES_set_decrypt_key((const unsigned char *)key_str.c_str(),
                /*key_str.size() * sizeof(char)*/AES_KEY_SIZE, &key_value);
    }
}

size_t StandardEncryptionManager::Encrypt(char *output, const char *input,
        size_t size, const String &key, const String &ivec)
{
    AES_KEY aes_key;
    memset(&aes_key, 0, sizeof(AES_KEY));
    GetAESKey(aes_key, key, AES_ENCRYPT);

    unsigned char *ivec_buffer = new unsigned char[ivec.size()];
    strcpy((char *)ivec_buffer, ivec.c_str());

    AES_cbc_encrypt((unsigned char *)input, (unsigned char *)output, size,
            &aes_key, ivec_buffer, AES_ENCRYPT);

    LOG("input=%s", input);
    LOG("output=%s", output);

    delete[] ivec_buffer;

    // XXX Is this correct?
    return size;
}

size_t StandardEncryptionManager::Decrypt(char *output, const char *input,
        size_t size, const String &key, const String &ivec)
{
    AES_KEY aes_key;
    GetAESKey(aes_key, key, AES_DECRYPT);

    unsigned char *ivec_buffer = new unsigned char[ivec.size()];
    strcpy((char *)ivec_buffer, ivec.c_str());

    AES_cbc_encrypt((unsigned char *)input, (unsigned char *)output, size,
            &aes_key, ivec_buffer, AES_DECRYPT);

    LOG("input=%s", input);
    LOG("output=%s", output);

    delete[] ivec_buffer;

    // XXX Is this correct?
    return size;
}

size_t StandardEncryptionManager::GetKeySize()
{
    return AES_KEY_SIZE;
}

size_t StandardEncryptionManager::GetIVSize()
{
    return AES_KEY_SIZE;
}


