#include "event.h"

#include <charconv>
#include <stdexcept>
#include <string>
#include <utility>
#include "copy_stats.h"

namespace nano_edr {

EventType EventTypeFromString(const std::string& type){
    if(type == "process_start") return EventType::kProcessStart;
    if(type == "process_end") return EventType::kProcessEnd;
    if(type == "file_create") return EventType::kFileCreate;
    if(type == "file_write") return EventType::kFileWrite;
    if(type == "file_delete") return EventType::kFileDelete;
    if(type == "file_move") return EventType::kFileMove;
    if(type == "net_connect") return EventType::kNetConnect;
    return EventType::kOther;
}

Timestamp Event::Validate(const std::string& raw_ts, const std::string& type){
    if(raw_ts.empty()) throw std::invalid_argument("Event: ts пуст");
    if(type.empty()) throw std::invalid_argument("Event: type пуст");

    uint64_t ms = 0;
    const char* first = raw_ts.data();
    const char* last = first + raw_ts.size();
    auto [ptr, ec] = std::from_chars(first, last, ms);
    
    if (ec != std::errc{} || ptr != last) {
        throw std::invalid_argument("Event: invalid ts " + raw_ts);
    }
    return Timestamp{ms};
}

Event::Event(const EventParts& parts){
    ts_ = Validate(parts.ts, parts.type); 
    event_type_ = EventTypeFromString(parts.type);

    uint64_t strings_count = 3 + 2 * parts.fields.size();
        
    raw_ts_ = parts.ts;
    type_ = parts.type;
    pid_ = parts.pid;
    fields_ = parts.fields;

    copy_stats().event_strings_copied += strings_count;
}

Event::Event(EventParts&& parts){
    ts_ = Validate(parts.ts, parts.type);
    event_type_ = EventTypeFromString(parts.type);

    uint64_t strings_count = 3 + 2 * parts.fields.size();

    raw_ts_ = std::move(parts.ts);
    type_ = std::move(parts.type);
    pid_ = std::move(parts.pid);
    fields_ = std::move(parts.fields);

    copy_stats().event_strings_moved += strings_count;
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