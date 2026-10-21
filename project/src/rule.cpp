#include "rule.h"

namespace nano_edr{

RuleBase::RuleBase(const std::string& id, Severity severity)
    : id_(id), severity_(severity) {}

void RuleBase::AddAction(const std::string& action){
    actions_.push_back(action);
}

MatchRule::MatchRule(const std::string& id, Severity severity)
    : RuleBase(id, severity) {}

void MatchRule::AddCondition(std::unique_ptr<ICondition> condition) {
    conditions_.push_back(std::move(condition));
}

bool MatchRule::Check(const Event& event){
    for(const auto& c : conditions_) {
        if (!c->Matches(event)) return false;
    }
    CountHit();
    return true;
}

SequenceRule::SequenceRule(const std::string& id, Severity severity, uint64_t window_ms)
    :RuleBase(id, severity), window_ms_(window_ms) {}

void SequenceRule::SetFirst(std::unique_ptr<ICondition> c) {first_ = std::move(c);}

void SequenceRule::SetSecond(std::unique_ptr<ICondition> c) {second_ = std::move(c);}

bool SequenceRule::Check(const Event& event) {
    if (!first_ || !second_) return false;

    const int64_t now = event.ts().ms;
    pending_.erase(
        std::remove_if(pending_.begin(), pending_.end(),
            [&](const Pending& p) { return now - p.ts_ms > window_ms_; }),
        pending_.end());

    if (second_->Matches(event)) {
        for (auto it = pending_.begin(); it != pending_.end(); ++it) {
            if (!same_pid_ || it->pid == event.pid()) {
                pending_.erase(it);
                CountHit();
                return true;
            }
        }
    }

    if (first_->Matches(event)) {
        pending_.push_back(Pending{event.ts().ms, event.pid()});
    }

    return false;
}

}