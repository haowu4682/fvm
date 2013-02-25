
#include <sstream>
#include <vector>

#include <common/accessinfo.h>
#include <common/util.h>

bool AccessList::IsIncluded(const String& username, const String& groupname) const
{
    Map<String, Set<String> >::const_iterator it = group_map_.find(groupname);
    if (it == group_map_.end()) {
        return false;
    }

    const Set<String>& user_set = it->second;
    return user_set.count(username);
}

void AccessList::AddUser(const String& username, const String& groupname)
{
    group_map_[groupname].insert(username);
}

void AccessList::RemoveUser(const String& username, const String& groupname)
{
    group_map_[groupname].erase(username);
}

String AccessList::ToString() const
{
    std::ostringstream os;

    ToStream(os);

    return os.str();
}

void AccessList::ToStream(std::ostream& os) const
{
    for (Map<String, Set<String> >::const_iterator it = group_map_.begin();
            it != group_map_.end(); ++it) {
        os << it->first;
        for (Set<String>::const_iterator jt = it->second.begin();
                jt != it->second.end(); ++jt) {
            os << ' ' << *jt;
        }
        os << std::endl;
    }
}

void AccessList::FromString(const String& input)
{
    std::istringstream is(input);

    FromStream(is);
}

void AccessList::FromStream(std::istream& input_stream)
{
    Vector<String> input_str_array;

    //for (size_t i = 0; i < input_str_array.size(); ++i) {
    while(input_stream) {
        readline(input_stream, input_str_array);

        if (!input_str_array.empty()) {

            for (size_t i = 1; i < input_str_array.size(); ++i) {
                group_map_[input_str_array[0]].insert(input_str_array[i]);
            }
        }
    }
}

