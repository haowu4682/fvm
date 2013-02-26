#ifndef KEY_MANAGER_H__
#define KEY_MANAGER_H__

#include <string>

#include <common/common.h>

class GroupKeyManager {
    public:
        virtual String GetKeyFileName(const String &key_name);

        virtual String GetKeyContent(const String &file_content);

    protected:
        String username;

    private:
        String user_private_key;
        String user_public_key;
};


#endif // KEY_MANAGER_H__

