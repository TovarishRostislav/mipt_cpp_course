#include "agent.h"

#include <print>
#include <string>

#include "agent_rules.h"
#include "rules.h"

namespace nano_edr {

Agent::Agent(size_t window_size, bool quiet)
    : window_(window_size), quiet_(quiet), engine_(BuildAgentRules()) {}

void Agent::HandleEvent(const Event& event) {
    ++total_events_;
    type_counts_[event.type()]++;

    std::vector<const IRule*> fired;
    engine_.ProcessEvent(event, &fired);

    for(const auto& rule : fired){
        std::print("[DETECT] {}  {}  ts={} pid={}\n",
                   SeverityName(rule->severity()),
                   rule->id(),
                   event.raw_ts(),
                   event.pid());
    }

    if(!fired.empty() && !quiet_){
        PrintContext();
    }

    const EventNode* old_tail = window_.back();
    window_.PushBack(event);

    if (window_.size() == 1) {
        ctx_ = window_.head();
    } else {
        ctx_ = old_tail;
    }
}   


void Agent::OnSkipped() {
    ++skipped_; 
}

void Agent::OnBroken() {
    ++broked_;  
}

void Agent::OnTick() {

}

void Agent::PrintSummary() const {
    if (quiet_) return;

    std::print("событий всего: {}\n", total_events_);
    for (const auto& [type, count] : type_counts_) {
        std::print("type={}: {}\n", type, count);
    }

    std::print("пропущено: {}, битых: {}\n", skipped_, broked_);

    std::print("правила, детектов:\n");
    for(size_t i = 0; i < engine_.size(); ++i){
        std::print("  {:<22} {}\n", engine_.rule(i).id(), engine_.rule(i).hits());
    }
}

void Agent::PrintContext() const {
    if (ctx_ == nullptr) return;

    if (ctx_->next == nullptr) {
        std::print("[CTX] -1: ts={} type={} pid={}\n",
                ctx_->event.raw_ts(), ctx_->event.type(), ctx_->event.pid());
    } else {
        std::print("[CTX] -2: ts={} type={} pid={}\n",
                ctx_->event.raw_ts(), ctx_->event.type(), ctx_->event.pid());
        std::print("[CTX] -1: ts={} type={} pid={}\n",
                ctx_->next->event.raw_ts(),
                ctx_->next->event.type(),
                ctx_->next->event.pid());
    }
}

}