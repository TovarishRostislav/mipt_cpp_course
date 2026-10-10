#include <fstream>
#include <print>
#include <string>
#include <map>
#include <charconv>
#include <cstring>
#include <exception>    
#include <stdexcept>


#include "parse.h"
#include "event.h"
#include "event_list.h"
#include "rules.h"       
#include "agent_rules.h"   
#include "os_handle.h"

class Agent{
public:
    Agent(std::size_t window_size, bool quiet):window_(window_size), quiet_(quiet){}

    void HandleEvent(const nano_edr::Event& event){
        ++total_events_;
        type_counts_[event.type()]++;

        const size_t detected = nano_edr::CheckRules(event, nano_edr::AgentRules(), nano_edr::AgentRuleCount());

        if (detected > 0 && !quiet_){
            PrintContext();
        }
        const nano_edr::EventNode* old_tail = window_.back();
        window_.PushBack(event);

        ctx_ = old_tail ? old_tail : window_.head();
    }

    void PrintSummary() const{
        if(quiet_) return;

        std::print("событий всего: {}\n", total_events_);
        for(const auto& [type,count] : type_counts_){
            std::print("type={}: {}\n", type, count);
        }
    }

    static void Trampoline(const os_event* ev, void* ctx){
        try{
        Agent* agent = static_cast<Agent*>(ctx);

        nano_edr::EventParts parts;
        parts.ts = std::to_string(ev->ts);
        parts.type = ev->type;
        parts.pid = (ev->pid == 0) ? std::string{} : std::to_string(ev->pid);

        for(size_t i = 0; i < ev->field_count; ++i){
            parts.fields.push_back({ev->fields[i].key, ev->fields[i].value});
        }

        nano_edr::Event event(parts);
        agent->HandleEvent(event);
    }catch(...){
        //скип по тз
    }
    }

private:
    void PrintContext() const{
        if (ctx_ == nullptr) return;
        
        if(ctx_->next == nullptr){
            std::print("[CTX] -1: ts={} type={} pid={}\n",
                ctx_->event.raw_ts(),
                ctx_->event.type(),
                ctx_->event.pid());
        }else{
            std::print("[CTX] -2: ts={} type={} pid={}\n", 
                ctx_->event.raw_ts(), 
                ctx_->event.type(),
                ctx_->event.pid());
            std::print("[CTX] -1: ts={} type={} pid={}\n",
                ctx_->next->event.raw_ts(),
                ctx_->next->event.type(),
                ctx_->next->event.pid());
        }
    }


    nano_edr::EventList window_;
    bool quiet_;
    uint64_t total_events_ = 0;
    std::map<std::string, int> type_counts_;
    const nano_edr::EventNode* ctx_ = nullptr;

};

class FileSource{
public:
    explicit FileSource(const std::string& path) : path_(path) {}

    void Run(Agent* agent){
        std::ifstream log(path_);
        if(!log){
            throw std::runtime_error("не удалось открыть файл: " + path_);  
        }

        std::string line;
        while (std::getline(log, line)){
            if(nano_edr::IsBlankOrComment(line)) continue;

            nano_edr::EventParts parts;
            if(!nano_edr::ParseEventParts(line, &parts)) continue;

            try{
                nano_edr::Event event(parts);
                agent->HandleEvent(event);
            }catch (const std::exception&){
                //скип по ТЗ
            }
        }
    }
private:
    std::string path_;
};

class OsSource{
public:
    explicit OsSource(const std::string& config_path) : handle_(config_path) {}

    void Run(Agent* agent){
        handle_.Subscribe(Agent::Trampoline, agent);
        handle_.Start();

        while(true){
            const os_status st = handle_.Wait(100);

            if(st == OS_TIMEOUT) continue;
            if (st == OS_OK) break;
            if (st == OS_STOPPED) break;

            throw std::runtime_error(
                std::string("os_wait failed: ") + os_status_str(st));
        }
    }
private:
    nano_edr::OsHandle handle_;
};

static int parse_args(int argc, char** argv,
                      std::size_t& window_size,
                      bool& quiet,
                      std::string& config_path,
                      bool& use_file) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--quiet") == 0) {
            quiet = true;
        } else if (std::strcmp(argv[i], "--window-size") == 0) {
            if (i + 1 >= argc) {
                std::print(stderr, "не передан --window-size\n");
                return 2;
            }
            const char* p = argv[++i];
            const char* end = p + std::strlen(p);
            auto [ptr, ec] = std::from_chars(p, end, window_size);
            if (ec != std::errc{} || ptr != end) {
                std::print(stderr, "неправильный --window-size\n");
                return 2;
            }
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
        std::print(stderr, "используй: nano-edr [--file] <журнал.log> [--quiet] [--window-size N]\n");
        return 2;
    }

    return 0;
}


int main(int argc, char** argv) {
    std::size_t window_size = 64;
    bool quiet = false;
    bool use_file = false;
    std::string config_path;

    int parse_res = parse_args(argc, argv, window_size, quiet, config_path, use_file);
    if(parse_res != 0) return parse_res;
    
    try{
        Agent agent(window_size, quiet);

        if(use_file){
            FileSource source(config_path);
            source.Run(&agent);
        }else{
            OsSource source(config_path);
            source.Run(&agent);
        }

        agent.PrintSummary();
        return 0;   

    }catch (const std::exception& error){
         std::print(stderr, "ошибка: {}\n", error.what());
        return 1;
    }
    

}
