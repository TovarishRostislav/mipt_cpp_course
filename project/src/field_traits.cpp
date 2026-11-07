#include "field_traits.h"
#include "text.h"   
#include <charconv>  

namespace nano_edr {

bool FieldTraits<std::string>::Parse(const std::string& text, std::string* out) {
    *out = text;
    return true;
}
bool FieldTraits<uint64_t>::Parse(const std::string& text, uint64_t* out) {
    if (text.empty()) {
        return false;
    }

    uint64_t value = 0;
    const char* first = text.data();
    const char* last = text.data() + text.size();

    auto [ptr, ec] = std::from_chars(first, last, value);

    if (ec == std::errc{} && ptr == last) {
        *out = value;
        return true;
    }
    return false;
}

bool FieldTraits<bool>::Parse(const std::string& text, bool* out) {
    if (EqualsIgnoreCase(text, "1") || 
        EqualsIgnoreCase(text, "true") || 
        EqualsIgnoreCase(text, "yes")) {
        *out = true;
        return true;
    }
    
    if (EqualsIgnoreCase(text, "0") || 
        EqualsIgnoreCase(text, "false") || 
        EqualsIgnoreCase(text, "no")) {
        *out = false;
        return true;
    }

    return false;
}

} 
