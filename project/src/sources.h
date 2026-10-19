#pragma once

#include <string>
#include <fstream>

#include "agent.h"
#include "os_handle.h"

namespace nano_edr {

class FileSource {
 public:
    explicit FileSource(const std::string& path);
    void Run(Agent* agent);

 private:
    std::string path_;
    std::ifstream log_;
};

class OsSource {
 public:
    explicit OsSource(const std::string& config_path);
    void Run(Agent* agent);

 private:
    OsHandle handle_;
};

}
 