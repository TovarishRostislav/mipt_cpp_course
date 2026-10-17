#include "fields.h"
#include "text.h"

#include <stdexcept>   
#include <cstdint>     
#include <cctype>      
#include <string>
#include <vector>
#include <charconv>    
#include <system_error>   

namespace nano_edr {

const std::string* FindField(const Event& event, const std::string& key) {
    if(key == "ts") return &event.raw_ts();
    if (key == "type") return &event.type();
    if(key == "pid") return &event.pid();

    for(const Field& field : event.fields()){
        if (field.key == key) return &field.value;
    }
    return nullptr;
}


const std::string& GetRequiredField(const Event& event, const std::string& key) {
    const std::string* found = FindField(event, key);
    if(!found){
        throw std::invalid_argument("отсутствует нужное поле: " + key);
    }
    return *found;
}

bool GetIntField(const Event& event, const std::string& key, uint64_t* out) {
    const std::string* found = FindField(event, key);
    if (!found) return false;

    const char* first = found->data();
    const char* last = found->data() + found->size();
    uint64_t value = 0;
    auto [ptr,ec] = std::from_chars(first, last, value);

    if (ec != std::errc{} || ptr != last) return false;

    *out = value;
    return true;
}


uint64_t GetIntField(const Event& event, const std::string& key, uint64_t fallback) {
    uint64_t value = 0;
    if(!GetIntField(event, key, &value)) return fallback;
    return value;
}


bool IsProcessStart(const Event& event) {
    return event.type() == "process_start";
}

bool IsFileWrite(const Event& event) {
    return event.type() == "file_create" || event.type() == "file_write"  || event.type() == "file_move";
}

bool IsNetConnect(const Event& event) {
    return event.type() == "net_connect";
}


bool PathEndsWith(const Event& event, const std::string& suffix) {
    const std::string* path = FindField(event, "path");
    if(!path) return false;

    return EndsWithIgnoreCase(*path, suffix);
}


bool CommandLineContains(const Event& event, const std::string& needle) {
    const std::string* cmdl = FindField(event, "cmdline");
    if (!cmdl) return false;

    std::string norm_cmdl = NormalizePath(*cmdl);
    std::string norm_needle = NormalizePath(needle);

    return norm_cmdl.find(norm_needle) != std::string::npos;
}


std::string NormalizePath(const std::string& path) {
    std::string lowd;
    bool last_backslash = false;
    lowd.reserve(path.size());
    for (char c : path) {
    char low = MyToLower(c);

    if (low == '/' || low == '\\') {
        if (!last_backslash) {
            lowd.push_back('\\');
            last_backslash = true;
        }

    } else {
        lowd.push_back(low);
        last_backslash = false;       
    }
}   
    auto replace_all = [](std::string& s, const std::string& from, const std::string& to){
        size_t pos = 0;
        while ((pos = s.find(from, pos)) != std::string::npos){
            s.replace(pos, from.size(), to);
            pos += to.size();
        }
    };
    
    replace_all(lowd, "%temp%", "\\appdata\\local\\temp");
    replace_all(lowd, "%tmp%",  "\\appdata\\local\\temp");

    return lowd;

}
}  
