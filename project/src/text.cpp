#include "text.h"

#include <cstddef>

namespace nano_edr{
bool EqualsIgnoreCase(const std::string& a, const std::string& b){
    if(a.size() != b.size()) return false;

    for(size_t i = 0; i < a.size(); ++i){
        if(MyToLower(a[i]) != MyToLower(b[i])) return false;
    }
    return true;
}

bool ContainsIgnoreCase(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    if (needle.size() > haystack.size()) return false;

    for (std::size_t i = 0; i + needle.size() <= haystack.size(); ++i) {
        bool match = true;
        for (std::size_t j = 0; j < needle.size(); ++j) {
            if (MyToLower(haystack[i + j]) != MyToLower(needle[j])) {
                match = false;
                break;
            }
        }
        if (match) return true;
    }
    return false;
}

bool EndsWithIgnoreCase(const std::string& haystack, const std::string& suff){
    if(suff.size() > haystack.size()) return false;

    const size_t offset = haystack.size() - suff.size();
    for(size_t i = 0; i < suff.size(); ++i){
        if (MyToLower(haystack[offset+i]) != MyToLower(suff[i])) return false;
    }
    return true;
}

}

