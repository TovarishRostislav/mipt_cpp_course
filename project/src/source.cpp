#include "sources.h"

#include <fstream>
#include <stdexcept>
#include <print>

#include "parse.h"

namespace nano_edr {

FileSource::FileSource(const std::string& path) : path_(path), log_(path) {
    if (!log_) {
        throw std::runtime_error("open error: " + path_);
    }
}
void FileSource::Run(IEventHandler* handler) {
    std::string line;
    while (std::getline(log_, line)) {
        try {
            if (IsBlankOrComment(line)){
                handler -> OnSkipped();
                continue;
            }

            EventParts parts;
            if (!ParseEventParts(line, &parts)){
                handler -> OnBroken();
                continue;
            }

        
            Event event(parts);
            handler->HandleEvent(event);
        } catch (const std::exception& ex) {
            std::print(stderr, "skip event: {}\n", ex.what());
            handler->OnBroken();
        } catch (...) {
            std::print(stderr, "skip event: неизвестная ошибка\n");
            handler->OnBroken();
        }
    }
}

OsSource::OsSource(const std::string& config_path)
    : handle_(config_path) {}

void OsSource::Run(IEventHandler* handler) {
    handle_.Subscribe(&OsSource::Trampoline, handler);
    handle_.Start();

    while (true) {
        const os_status st = handle_.Wait(100);

        if (st == OS_TIMEOUT){
            handler->OnTick();
            continue;
        }
        if (st == OS_OK) break;
        if (st == OS_STOPPED) break;

        throw std::runtime_error(
            std::string("os_wait failed: ") + os_status_str(st));
    }
}
void OsSource::Trampoline(const os_event* ev, void* ctx) noexcept {
    IEventHandler* handler = static_cast<IEventHandler*>(ctx);

    try {
        EventParts parts;
        parts.ts   = std::to_string(ev->ts);
        parts.type = ev->type;
        parts.pid  = (ev->pid == 0) ? std::string{} : std::to_string(ev->pid);
        for (std::size_t i = 0; i < ev->field_count; ++i) {
            parts.fields.push_back({ev->fields[i].key, ev->fields[i].value});
        }

        Event event(parts);
        handler->HandleEvent(event);
    } catch (const std::exception& ex) {
        std::print(stderr, "trampoline: {}\n", ex.what());
        handler->OnBroken();
    } catch (...) {
        std::print(stderr, "trampoline: undefined error\n");
        handler->OnBroken();
    }
}
}  