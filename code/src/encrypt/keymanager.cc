
#include <cstring>

#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <encrypt/keymanager.h>

// XXX We only support PEM format for now!
void KeyManager::ReadPublicKeyFromFile(const String &filename)
{
    FILE *file = fopen(filename.c_str(), "r");
    // XXX Check this sentence if reading breaks!
    //PEM_read_RSAPublicKey(file, &user_public_key_, NULL, NULL);
    PEM_read_RSA_PUBKEY(file, &user_public_key_, NULL, NULL);
}

void KeyManager::ReadPrivateKeyFromFile(const String &filename)
{
    FILE *file = fopen(filename.c_str(), "r");
    // XXX Check this sentence if reading breaks!
    PEM_read_RSAPrivateKey(file, &user_private_key_, NULL, NULL);
}

String KeyManager::GetGroupKeyFileName(const String &group_name)
{
    return group_name + "." + username_ + ".key";
}

String KeyManager::GetGroupKeyContent(const String &file_content)
{
    int rc;
    unsigned char *in_buf = new unsigned char[file_content.size()];
    unsigned char *out_buf = new unsigned char[file_content.size()];

    // TODO Check whether removing "const" constraint is safety, if so we can
    // save the time for copying the content
    memcpy((char *)in_buf, file_content.c_str(), file_content.size());

    rc = RSA_private_decrypt(file_content.size(), in_buf,
            out_buf, user_private_key_, RSA_PKCS1_OAEP_PADDING);
    if (rc < 0) {
        LOG("Group key decryption fails for %s.", file_content.c_str());
        out_buf[0] = '\0';
    }

out:
    String ret((char *)out_buf);
    delete[] in_buf;
    delete[] out_buf;

    return ret;
}

String KeyManager::GetNewGroupKeyFileContent(const String &key_content)
{
    int rc;
    size_t size = key_content.size();
    unsigned char *in_buf = new unsigned char[size];
    // XXX Magic number
    unsigned char *out_buf = new unsigned char[10000];

    // TODO Check whether removing "const" constraint is safety, if so we can
    // save the time for copying the content
    strncpy((char *)in_buf, key_content.c_str(), size);

    rc = RSA_public_encrypt(size, in_buf,
            out_buf, user_public_key_, RSA_PKCS1_OAEP_PADDING);
    if (rc < 0) {
        LOG("Group key encryption fails for %s.", key_content.c_str());
        out_buf[0] = '\0';
    }

out:
    String ret((char *)out_buf, rc);
    delete[] in_buf;
    delete[] out_buf;

    return ret;
}

