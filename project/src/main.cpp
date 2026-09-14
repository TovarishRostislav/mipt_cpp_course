#include <cstdio>
#include <fstream>
#include <print>
#include <string>
#include <map>
#include <vector>

int main(int argc, char** argv) {
    bool quiet = false;
    std::string log_path;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--quiet") {
            quiet = true;
        } else {
            log_path = arg;
        }
    }

    if (log_path.empty()) {
        std::print(stderr, "использование: nano-edr <журнал.log> [--quiet]\n");
        return 2;
    }

    std::ifstream log(log_path);
    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", log_path);
        return 2;
    }

    long long lines = 0;
    long long comments = 0;
    long long total_events = 0;
    std::string line;
    std::map<std::string, int> type_counts;

    const std::vector<std::string> patterns = {
        "wscript.exe",
        ".locked",
        "certutil.exe",
        "\\Startup\\"
    };

    while (std::getline(log, line)) {
        ++lines;

        size_t first = line.find_first_not_of(" \t");
        bool is_empty = (first == std::string::npos);
        bool is_comment = false;
        if (!is_empty && (line[first] == '#' || line[first] == ';')) {
            is_comment = true;
        }

        if (is_empty || is_comment) {
            if (is_comment) {
                ++comments;
            }
            continue;
        }

        ++total_events;

        size_t pos = line.find("type=");
        if (pos != std::string::npos) {
            size_t start = pos + 5;
            size_t end = line.find(' ', start);
            std::string type = line.substr(start, end - start);
            type_counts[type]++;
        }

        for (const auto& pat : patterns) {
            if (line.find(pat) != std::string::npos) {
                std::print("[DETECT] строка {}, признак {}: {}\n",
                           lines, pat, line);
            }
        }
    }

    if (!quiet) {
        std::print("строк {}, из них комментариев {}\n", lines, comments);
        std::print("событий всего: {}\n", total_events);
        for (const auto& [type, count] : type_counts) {
            std::print("type={}: {}\n", type, count);
        }
    }

    return 0;
}