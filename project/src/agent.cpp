#include "agent.h"

#include <print>
#include <string>

#include "agent_rules.h"
#include "rules.h"

namespace nano_edr {

Agent::Agent(size_t window_size, bool quiet)
    : window_(window_size), quiet_(quiet), rules_(BuildAgentRules()) {}

void Agent::HandleEvent(const Event& event) {
    ++total_events_;
    type_counts_[event.type()]++;

    bool any_detected = false;

    for (MatchRule& rule : rules_){
        if (rule.Check(event)){
            std::print("[DETECT] {}  {}  ts={} pid={}\n",
                    SeverityName(rule.severity()),
                    rule.id(),
                    event.raw_ts(),
                    event.pid());
            any_detected = true;
        }
    }

    if (any_detected && !quiet_) {
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

void Agent::PrintSummary() const {
    if (quiet_) return;

    std::print("событий всего: {}\n", total_events_);
    for (const auto& [type, count] : type_counts_) {
        std::print("type={}: {}\n", type, count);
    }

    std::print("правила, детектов:\n");
    for (const MatchRule& rule : rules_){
        std::print("  {:<22} {}\n", rule.id(), rule.hits());
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

void Agent::Trampoline(const os_event* ev, void* ctx) noexcept {
    try {
        Agent* agent = static_cast<Agent*>(ctx);

        EventParts parts;
        parts.ts   = std::to_string(ev->ts);
        parts.type = ev->type;
        parts.pid  = (ev->pid == 0) ? std::string{} : std::to_string(ev->pid);
        for (std::size_t i = 0; i < ev->field_count; ++i) {
            parts.fields.push_back({ev->fields[i].key, ev->fields[i].value});
        }

        Event event(parts);
        agent->HandleEvent(event);
    } catch (const std::exception& ex) {
        std::print(stderr, "trampoline: {}\n", ex.what());
    } catch (...) {
        std::print(stderr, "trampoline: undefined error\n");
    }
}

}