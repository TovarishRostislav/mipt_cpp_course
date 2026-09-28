#include "fields.h"

#include <stdexcept>   
#include <cstdint>     
#include <cctype>      
#include <string>
#include <vector>
#include <charconv>    
#include <system_error>   

namespace nano_edr {


const std::string* FindField(const Event& event, const std::string& key) {
    for(const Field& field : event.fields){
        if(field.key == key){
            return &field.value;
        }
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
    return event.type == "process_start";
}

bool IsFileWrite(const Event& event) {
    return event.type == "file_create" || event.type == "file_write"  || event.type == "file_move";
}

bool IsNetConnect(const Event& event) {
    return event.type == "net_connect";
}


bool PathEndsWith(const Event& event, const std::string& suffix) {
    const std::string* path = FindField(event, "path");
    if(!path) return false;

    std::string norm_path = NormalizePath(*path);
    std::string norm_suff = NormalizePath(suffix);

    if(norm_path.size() < norm_suff.size()) return false;

    return norm_path.ends_with(norm_suff);


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
    lowd.reserve(path.size());
    for(char c : path){
        char low = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if(low == '/'){
            lowd.push_back('\\');
        }else{
            lowd.push_back(low);
        }
    }
    std::string conected;
    conected.reserve(lowd.size());
    bool last_backslash = false;
    for(char c : lowd){
        if(c == '\\'){
            if(!last_backslash){
                conected.push_back(c);
            }
            last_backslash = true;
        }else{
            conected.push_back(c);
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
    replace_all(conected, "%temp%", "\\appdata\\local\\temp");
    replace_all(conected, "%tmp%",  "\\appdata\\local\\temp");

    return conected;
}

}  