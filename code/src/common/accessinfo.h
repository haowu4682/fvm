#ifndef COMMON_ACCESSINFO_H__
#define COMMON_ACCESSINFO_H__

#include <iostream>
#include <map>
#include <set>

#include <common/common.h>

class AccessInfo {
    public:
        bool IsIncluded(const String& username, const String& groupname);
        void AddUser(const String& username, const String& groupname);
        void RemoveUser(const String& username, const String& groupname);

        String ToString() const;
        void FromString(const String& input);
        void FromStream(std::istream& input_stream);

    private:
        Map<String, Set<String> > group_map_;
};

#endif // COMMON_ACCESSINFO_H__

