#include <charconv>
#include <cstring>
#include <exception>
#include <print>
#include <string>

#include "agent.h"
#include "sources.h"

static int parse_args(int argc, char** argv,
                      bool& quiet,
                      std::string& config_path,
                      bool& use_file) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--quiet") == 0) {
            quiet = true;
        } else if (std::strcmp(argv[i], "--file") == 0) {
            if (i + 1 >= argc) {
                std::print(stderr, "после --file нет значения\n");
                return 2;
            }
            if (!config_path.empty()) {
                std::print(stderr, "путь задан дважды\n");
                return 2;
            }
            use_file = true;
            config_path = argv[++i];
        } else {
            if (!config_path.empty()) {
                std::print(stderr, "путь задан дважды\n");
                return 2;
            }
            config_path = argv[i];
        }
    }

    if (config_path.empty()) {
        std::print(stderr, "используй: nano-edr [--file] <журнал.log> [--quiet]\n");
        return 2;
    }

    return 0;
}

int main(int argc, char** argv) {
    bool quiet = false;
    bool use_file = false;
    std::string config_path;

    int parse_res = parse_args(argc, argv, quiet, config_path, use_file);
    if (parse_res != 0) return parse_res;

    try {
        nano_edr::Agent agent(quiet);

        std::unique_ptr<nano_edr::IEventSource> source;

        if (use_file) {
            source = std::make_unique<nano_edr::FileSource>(config_path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(config_path);
        }

        source->Run(&agent);
        agent.PrintSummary();
        return 0;
    } catch (const std::exception& error) {
        std::print(stderr, "error: {}\n", error.what());
        return 1;
    }
}