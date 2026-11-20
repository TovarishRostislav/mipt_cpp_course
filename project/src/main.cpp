#include <cstring>
#include <exception>
#include <print>
#include <string>
#include <memory>
#include <iostream>
#include <utility>

#include "agent.h"
#include "sources.h"
#include "alert_sink.h"
#include "edr_handle.h"
#include "field_traits.h"
#include "copy_stats.h"

struct Options{
    bool quiet = false;
    bool use_file = false;
    bool copies = false;
    bool dry_run = false;
    std::string path;
    std::string out;
};

static int parse_args(int argc, char** argv, Options& opts){
    for(int i = 1; i < argc; ++i){
        if(std::strcmp(argv[i], "--quiet") == 0){
            opts.quiet = true;
        }else if(std::strcmp(argv[i], "--file") == 0){
            if(i + 1 >= argc){
                std::print(stderr, "после --file нет значения\n");
                return 2;
            }
            if(!opts.path.empty()){
                std::print(stderr, "путь задан дважды\n");
                return 2;
            }
            opts.use_file = true;
            opts.path = argv[++i];
        }else if(std::strcmp(argv[i], "--out") == 0) {
            if(i + 1 >= argc) {
                std::print(stderr, "после --out нет значения\n");
                return 2;
            }
            opts.out = argv[++i];
        }else if(std::strcmp(argv[i], "--dry-run") == 0){
            opts.dry_run = true;
        }else if(std::strcmp(argv[i], "--copies") == 0){
            opts.copies = true;
        }else{
            if(!opts.path.empty()){
                std::print(stderr, "путь задан дважды\n");
                return 2;
            }
            opts.path = argv[i];
        }
    }

    if (opts.path.empty()) {
        std::print(stderr, "используй: nano-edr [--file] <журнал.log> [--quiet] [--out <файл>] [--copies] [--dry-run]\n");
        return 2;
    }

    return 0;
}

int main(int argc, char** argv) {
    Options opts;

    int parse_res = parse_args(argc, argv, opts);
    if (parse_res != 0) return parse_res;

    try {
        nano_edr::EdrHandle edr(opts.path);
        const std::string text = edr.Config("dedup_period_ms", "10000");
        uint64_t dedup_period_ms = 10000;
        if (!nano_edr::FieldTraits<uint64_t>::Parse(text, &dedup_period_ms)) {
            std::print(stderr, "[WARN] dedup_period_ms не разбирается\n");
            dedup_period_ms = 10000;
        }

        std::unique_ptr<nano_edr::IEventSource> source;
        if (opts.use_file) {
            source = std::make_unique<nano_edr::FileSource>(opts.path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(opts.path);
        }

        nano_edr::Agent agent(opts.quiet, dedup_period_ms);

        nano_edr::FanOutSink targets;
        targets.Add(nano_edr::ConsoleSink());  
        if (!opts.out.empty()) {
            targets.Add(nano_edr::FileSink(opts.out));
        }
        targets.Add(nano_edr::EdrSink(&edr));

        agent.Subscribe(std::move(targets));

        agent.EnableResponse(source->os(), opts.dry_run, 
                             edr.Config("quarantine_dir", "C:\\quarantine"));

        source->Run(&agent);
        
        if (edr.Flush() != EDR_OK) {
            std::print(stderr, "[WARN] детекты не отправлены\n");
        }

        agent.PrintSummary();
        if (opts.copies) {
            const auto& stats = nano_edr::copy_stats();
            std::cout << "[COPY] строк события скопировано " << stats.event_strings_copied 
                    << ", перемещено " << stats.event_strings_moved << "\n";
            std::cout << "[COPY] записей WindowEntry скопировано " << stats.window_copied 
                    << ", перемещено " << stats.window_moved << "\n";
            std::cout << "[COPY] событий скопировано в детекты " << stats.evidence_copied << "\n";
            std::cout << "[COPY] копий всего " << stats.total() << "\n";
        }
        
        return 0;
    } catch (const std::exception& error) {
        std::print(stderr, "error: {}\n", error.what());
        return 1;
    }
}