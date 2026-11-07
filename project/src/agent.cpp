#include "agent.h"

#include <print>
#include <string>

#include "agent_rules.h"
#include "rules.h"

namespace nano_edr {

Agent::Agent(bool quiet)
    : quiet_(quiet), engine_(BuildAgentRules()) {}

void Agent::HandleEvent(const Event& event) {
    ++total_events_;
    type_counts_[event.type()]++;

    std::vector<const IRule*> fired;
    engine_.ProcessEvent(event, &fired);

    if(!fired.empty() && !quiet_){
        PrintContext();
    }

    for(const auto& rule : fired){
        std::print("[DETECT] {}  {}  ts={} pid={}\n",
                   SeverityName(rule->severity()),
                   rule->id(),
                   event.raw_ts(),
                   event.pid());
    }

    window_.PushBack(WindowEntry{event.ts(), event.type(), event.pid()});
}   


void Agent::OnSkipped() {
    ++skipped_; 
}

void Agent::OnBroken() {
    ++broken_;  
}

void Agent::OnTick() {

}

void Agent::PrintSummary() const {
    if (quiet_) return;

    std::print("событий всего: {}\n", total_events_);
    for (const auto& [type, count] : type_counts_) {
        std::print("type={}: {}\n", type, count);
    }

    std::print("пропущено: {}, битых: {}\n", skipped_, broken_);

    std::print("правила, детектов:\n");
    for(size_t i = 0; i < engine_.size(); ++i){
        std::print("  {:<22} {}\n", engine_.rule(i).id(), engine_.rule(i).hits());
    }

    std::print("[STAT] в окне {} из {} мест\n", window_.size(), RingBuffer<WindowEntry, Agent::kWindowCap>::capacity());
}

void Agent::PrintContext() const {
    if (window_.size() >= 2) {
        const auto& prev = window_[window_.size() - 2];
        std::print("[CTX] -2: {} {} pid={}\n", prev.ts.ms, prev.type, prev.pid);
        
        const auto& last = window_.back();
        std::print("[CTX] -1: {} {} pid={}\n", last.ts.ms, last.type, last.pid);
    } else if (window_.size() == 1) {
        const auto& last = window_.back();
        std::print("[CTX] -1: {} {} pid={}\n", last.ts.ms, last.type, last.pid);
    }
}

}