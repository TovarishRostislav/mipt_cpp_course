#include "event.h"

#include <charconv>
#include <stdexcept>
#include <string>

namespace nano_edr {

Event::Event(const EventParts& parts):
    raw_ts_(parts.ts),
    type_(parts.type),
    pid_(parts.pid),
    fields_(parts.fields){
        if (parts.ts.empty())   throw std::invalid_argument("Event: ts пуст");
        if (parts.type.empty()) throw std::invalid_argument("Event: type пуст");

        uint64_t ms = 0;
        const char* first = parts.ts.data();
        const char* last = first + parts.ts.size();
        auto [ptr, ec] = std::from_chars(first,last, ms);
        if (ec != std::errc{} || ptr != last) {
            throw std::invalid_argument("Event: invalid ts " + parts.ts);
        }

        ts_ = Timestamp{ms};
}

std::string ToString(const Event& event){
    std::string res;
    res += "ts=" + event.raw_ts();
    res += " type=" + event.type();
    res += " pid=" + event.pid();

    for (const Field& f : event.fields()){
        res += ' ' + f.key;
        res += '=' + f.value;
    }

    return res;
}

std::ostream& operator<<(std::ostream& out, const Event& event){
    return out << ToString(event);
}
}