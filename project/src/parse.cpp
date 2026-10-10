#include "parse.h"

namespace nano_edr {


bool IsBlankOrComment(const std::string& line) {
    
    std::size_t first = line.find_first_not_of(" \t");

    if(first == std::string::npos){
        return true;
    }
    char c = line[first];
    return c == '#' || c == ';';
}

bool ParseEventParts(const std::string& line, EventParts* out) {
    if (IsBlankOrComment(line)) return false;

    *out = EventParts{};

    bool has_ts = false, has_type = false, has_pid = false;

    const std::size_t n = line.size();
    std::size_t i = 0;                

    while (i < n) {

        while (i < n && (line[i] == ' ' || line[i] == '\t')) ++i;
        if (i >= n) break;

        
        std::size_t eq = line.find('=', i);
        if (eq == std::string::npos) return false;   

        
        for (std::size_t j = i; j < eq; ++j) {
            if (line[j] == ' ' || line[j] == '\t') return false;  
        }

        std::string key = line.substr(i, eq - i);
        if (key.empty()) return false;               

        i = eq + 1;
        std::string value;

        if (i < n && line[i] == '"') {
            ++i;                                    
            while (i < n && line[i] != '"') {
                value.push_back(line[i]);              
                ++i;
            }
            if (i >= n) return false;               
            ++i;                                    
            if (i < n && line[i] != ' ' && line[i] != '\t') {
                return false;                       
            }
        } else {
            while (i < n && line[i] != ' ' && line[i] != '\t') {
                value.push_back(line[i]);
                ++i;
            }
        }


        if (key == "ts" && !has_ts) {
            out->ts = value;   has_ts = true;
        } else if (key == "type" && !has_type) {
            out->type = value; has_type = true;
        } else if (key == "pid" && !has_pid) {
            out->pid = value;  has_pid = true;
        } else {
            out->fields.emplace_back(key, value);    
        }
    }

    return true;
}

}  