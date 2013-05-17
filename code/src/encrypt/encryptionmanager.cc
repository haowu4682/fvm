
#include <cstring>

#include <openssl/aes.h>

#include <encrypt/encryptionmanager.h>

template <size_t multiple> size_t round_up(const size_t len)
{
    if (len % multiple == 0)
        return len;
    else
        return ((len / multiple) + 1) * multiple;
}

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

    DBG("key=%s", key.c_str());
    DBG("iv=%s", ivec_buffer);
    DBG("size=%ld", size);
    AES_cbc_encrypt((unsigned char *)input, (unsigned char *)output, size,
            &aes_key, ivec_buffer, AES_ENCRYPT);

    DBG("input=%s", input);
    DBG("output=%s", output);

    delete[] ivec_buffer;

    return round_up<AES_BLOCK_SIZE>(size);
}

size_t StandardEncryptionManager::Decrypt(char *output, const char *input,
        size_t size, const String &key, const String &ivec)
{
    AES_KEY aes_key;
    memset(&aes_key, 0, sizeof(AES_KEY));
    GetAESKey(aes_key, key, AES_DECRYPT);

    unsigned char *ivec_buffer = new unsigned char[ivec.size()];
    strcpy((char *)ivec_buffer, ivec.c_str());

    DBG("key=%s", key.c_str());
    DBG("iv=%s", ivec_buffer);
    AES_cbc_encrypt((unsigned char *)input, (unsigned char *)output, size,
            &aes_key, ivec_buffer, AES_DECRYPT);

    DBG("input=%s", input);
    DBG("output=%s", output);
    DBG("size=%ld", size);

    delete[] ivec_buffer;

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


