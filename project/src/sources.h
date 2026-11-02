#pragma once

#include <fstream>
#include <string>

#include "event_source.h"
#include "os_handle.h"

namespace nano_edr {

class FileSource : public IEventSource {
 public:
    explicit FileSource(const std::string& path);
    void Run(IEventHandler* handler) override;

 private:
    std::string path_;
    std::ifstream log_;
};

class OsSource : public IEventSource{
 public:
    explicit OsSource(const std::string& config_path);
    void Run(IEventHandler* handler) override;

 private:
 static void Trampoline(const os_event* event, void* ctx) noexcept;
    OsHandle handle_;
};

}
 