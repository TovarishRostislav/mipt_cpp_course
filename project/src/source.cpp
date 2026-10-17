#include "sources.h"

#include <fstream>
#include <stdexcept>

#include "parse.h"

namespace nano_edr {

FileSource::FileSource(const std::string& path) : path_(path) {}

void FileSource::Run(Agent* agent) {
    std::ifstream log(path_);
    if (!log) {
        throw std::runtime_error("не удалось открыть файл: " + path_);
    }

    std::string line;
    while (std::getline(log, line)) {
        if (IsBlankOrComment(line)) continue;

        EventParts parts;
        if (!ParseEventParts(line, &parts)) continue;

        try {
            Event event(parts);
            agent->HandleEvent(event);
        } catch (...) {
            // пропускаем битое событие
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