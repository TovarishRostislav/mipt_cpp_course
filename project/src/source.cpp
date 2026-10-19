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
void FileSource::Run(Agent* agent) {
    std::string line;
    while (std::getline(log_, line)) {
        try {
            if (IsBlankOrComment(line)) continue;

            EventParts parts;
            if (!ParseEventParts(line, &parts)) continue;

        
            Event event(parts);
            agent->HandleEvent(event);
        } catch (const std::exception& ex) {
            std::print(stderr, "skip event: {}\n", ex.what());
        } catch (...) {
            std::print(stderr, "skip event: неизвестная ошибка\n");
        }
    }
}

OsSource::OsSource(const std::string& config_path)
    : handle_(config_path) {}

void OsSource::Run(Agent* agent) {
    handle_.Subscribe(Agent::Trampoline, agent);
    handle_.Start();

    while (true) {
        const os_status st = handle_.Wait(100);

        if (st == OS_TIMEOUT) continue;
        if (st == OS_OK) break;
        if (st == OS_STOPPED) break;

        throw std::runtime_error(
            std::string("os_wait failed: ") + os_status_str(st));
    }
}

}  