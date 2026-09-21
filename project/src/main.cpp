#include <fstream>
#include <print>
#include <string>
#include <map>
#include <vector>
#include <charconv>
#include <cstring>

#include "parse.h"
#include "event_list.h"


static void print_context(const nano_edr::Event* const last[2], int count) {
    for (int i = 0; i < count; ++i) {
        const nano_edr::Event* e = last[2 - count + i];
        int label = -(count - i);
        std::print("[CTX] {}: ts={} type={} pid={}\n",
                   label, e->ts, e->type, e->pid);
    }
}

int main(int argc, char** argv) {
    std::size_t window_size = 64;
    bool quiet = false;
    std::string log_path;


    const nano_edr::Event* last_two[2] = {nullptr, nullptr};
    int last_count = 0;


    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--quiet") == 0) {
            quiet = true;
        } else if (std::strcmp(argv[i], "--window-size") == 0) {
            if (i + 1 >= argc) {
                std::print(stderr, "не передан --window-size\n");
                return 1;
            }
            const char* p   = argv[++i];
            const char* end = p + std::strlen(p);
            auto [ptr, ec] = std::from_chars(p, end, window_size);
            if (ec != std::errc{} || ptr != end) {
                std::print(stderr, "неправильный --window-size\n");
                return 1;
            }
        } else {
            log_path = argv[i];
        }
    }

    if (log_path.empty()) {
        std::print(stderr, "используй: nano-edr <журнал.log> [--quiet] [--window-size N]\n");
        return 2;
    }

    std::ifstream log(log_path);
    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", log_path);
        return 2;
    }

    nano_edr::EventList window;
    window.capacity = window_size;

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

        if (nano_edr::IsBlankOrComment(&line)) {
            ++comments;
            continue;
        }

        nano_edr::Event ev;
        if (!nano_edr::ParseEventLine(&line, &ev)) {
            continue;
        }

        ++total_events;
        type_counts[ev.type]++;


        bool any_detected = false;
        for (const auto& pat : patterns) {
            bool hit = false;
            for (const auto& f : ev.fields) {
                if (f.value.find(pat) != std::string::npos) {
                    hit = true;
                    break;
                }
            }
            if (hit) {
                std::print("[DETECT] строка {}, признак {}: {}\n",
                           lines, pat, line);
                any_detected = true;
            }
        }


        if (any_detected && !quiet) {
            print_context(last_two, last_count);
        }

        nano_edr::ListPushBack(&window, &ev);


        last_two[0] = (window.size >= 2) ? last_two[1] : nullptr;
        last_two[1] = &window.tail->event;
        last_count  = (window.size >= 2) ? 2 : 1;
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