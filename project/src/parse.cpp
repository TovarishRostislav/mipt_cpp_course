#include "parse.h"

namespace nano_edr {

const char* ws = " \t";

bool IsBlankOrComment(const std::string* line) {
    if (line->empty()) {
        return true;
    }
    size_t pos = line->find_first_not_of(ws);
    if (pos == std::string::npos) {
        return true;
    }
    return (*line)[pos] == '#' || (*line)[pos] == ';';
}

bool ParseEventLine(const std::string* line, Event* out) {
    if (IsBlankOrComment(line)) return false;

    *out = Event{};

    bool has_ts = false, has_type = false, has_pid = false;

    const std::string& s = *line;                

    const std::size_t n = s.size();
    std::size_t i = 0;

    while (i < n) {

        while (i < n && (s[i] == ' ' || s[i] == '\t')) ++i;
        if (i >= n) break;

        
        std::size_t eq = s.find('=', i);
        if (eq == std::string::npos) return false;   

        
        for (std::size_t j = i; j < eq; ++j) {
            if (s[j] == ' ' || s[j] == '\t') return false;  
        }

        std::string key = s.substr(i, eq - i);
        if (key.empty()) return false;               

        i = eq + 1;
        std::string value;

        if (i < n && s[i] == '"') {
            ++i;                                    
            while (i < n && s[i] != '"') {
                value.push_back(s[i]);              
                ++i;
            }
            if (i >= n) return false;               
            ++i;                                    
            if (i < n && s[i] != ' ' && s[i] != '\t') {
                return false;                       
            }
        } else {
            while (i < n && s[i] != ' ' && s[i] != '\t') {
                value.push_back(s[i]);
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

    return has_ts && has_type;
}

}  