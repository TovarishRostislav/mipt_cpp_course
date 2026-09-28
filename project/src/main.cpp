#include <fstream>
#include <print>
#include <string>
#include <map>
#include <charconv>
#include <cstring>
#include <exception>    

#include "parse.h"
#include "event_list.h"
#include "rules.h"       
#include "agent_rules.h"   

static void print_context(const nano_edr::EventNode* ctx) {
    if (ctx == nullptr) return;  

    if (ctx->next == nullptr) { 
        std::print("[CTX] -1: ts={} type={} pid={}\n",
                   ctx->event.ts, ctx->event.type, ctx->event.pid);
    } else { 
        std::print("[CTX] -2: ts={} type={} pid={}\n",
                   ctx->event.ts, ctx->event.type, ctx->event.pid);
        std::print("[CTX] -1: ts={} type={} pid={}\n",
                   ctx->next->event.ts, ctx->next->event.type, ctx->next->event.pid);
    }
}

static void print_summary(long long lines, long long comments,
                        long long total_events, const std::map<std::string, int>& type_counts){
                            std::print("строк {}, из них комментариев {}\n", lines, comments);
                            std::print("событий всего: {}\n", total_events);
                            for(const auto& [type,count] : type_counts){
                                std::print("type={}: {}\n", type, count);
                            }
}

static bool parse_args(int argc, char** argv, std::size_t& window_size,
                    bool& quiet, std::string& log_path){
                        for(int i = 1; i< argc; ++i){
                            if(std::strcmp(argv[i], "--quiet") == 0){
                                quiet = true;
                            }else if(std::strcmp(argv[i], "--window-size") == 0){
                                if (i+1 >= argc){
                                    std::print(stderr, "не передан --window-size\n");
                                    return false;
                                }
                                const char* l = argv[++i];
                                const char* r = l + std::strlen(l);
                                auto [ptr, ec] = std::from_chars(l, r, window_size);
                                if(ec != std::errc{} || ptr != r){
                                    std::print(stderr, "неправильный --window-size\n");
                                    return false;
                                }
                            }else{
                                log_path = argv[i];
                            }
                        }
                        if(log_path.empty()){
                            std::print(stderr, "используй: nano-edr <журнал.log> [--quiet] [--window-size N]\n");
                            return false;
                        }
                        
    return true;
 }

static int process_log(std::ifstream& log, std::size_t window_size, bool quiet){
    nano_edr::EventList window;
    window.capacity = window_size;

    nano_edr::EventNode* ctx = nullptr; 

    long long lines = 0;
    long long comments = 0;
    long long total_events = 0;
    std::string line;
    std::map<std::string, int> type_counts;

    while(std::getline(log,line)){
        ++lines;

        if (nano_edr::IsBlankOrComment(&line)){
            ++comments;
            continue;
        }

        nano_edr::Event ev;
        if (!nano_edr::ParseEventLine(&line, &ev)){
            continue;
        }

        ++total_events;
        type_counts[ev.type]++;

        const std::size_t detected = nano_edr::CheckRules(ev,nano_edr::AgentRules(),
            nano_edr::AgentRuleCount());
        if (detected > 0 && !quiet){
            print_context(ctx);
        }

        if (window.capacity == 1){ 
            nano_edr::ListPushBack(&window, &ev);
            ctx = window.head;
        }else{
            nano_edr::EventNode* old_tail = window.tail;
            nano_edr::ListPushBack(&window, &ev);
            ctx = old_tail ? old_tail : window.head;
        }


    }

    if(!quiet){
        print_summary(lines, comments, total_events, type_counts);
    }
    return 0;
}


int main(int argc, char** argv) {
    std::size_t window_size = 64;
    bool quiet = false;
    std::string log_path;

    if (!parse_args(argc, argv, window_size, quiet, log_path)) return 2;

    std::ifstream log(log_path);
    if (!log){
        std::print(stderr, "не удалось открыть журнал: {}\n", log_path);
        return 2;
    }
    
    try{
        return process_log(log, window_size, quiet);    
    }catch (const std::exception& error){
         std::print(stderr, "ошибка: {}\n", error.what());
        return 1;
    }
    

}