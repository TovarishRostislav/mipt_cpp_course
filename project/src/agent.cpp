#include "agent.h"

#include <print>
#include <string>

#include "agent_rules.h"

namespace nano_edr {

Agent::Agent(bool quiet, uint64_t dedup_period_ms)
    : quiet_(quiet), dedup_period_ms_(dedup_period_ms),  engine_(BuildAgentRules(&model_)) {engine_.SetModel(&model_);}

void Agent::HandleEvent(const Event& event) {
    model_.Observe(event);
    if (responder_ != nullptr && responder_->Recognize(event)) {
        WindowEntry entry;
        entry.ts = event.ts(); entry.type = event.type(); entry.pid = event.pid();
        window_.PushBack(std::move(entry));
        return;
    }

    ++total_events_;
    type_counts_[event.type()]++;

    context_printed_ = false;

    engine_.ProcessEvent(event, nullptr);

    WindowEntry entry;
    entry.ts = event.ts();
    entry.type = event.type();
    entry.pid = event.pid();
    
    window_.PushBack(std::move(entry)); 

}   

void Agent::OnSkipped() {
    ++skipped_; 
}

void Agent::OnBroken() {
    ++broken_;  
}

void Agent::OnTick() {

}

void Agent::EnableResponse(OsHandle* os, bool dry_run, const std::string& quarantine_dir) {
    planner_ = std::make_unique<ResponsePlanner>(&model_, quarantine_dir);
    responder_ = std::make_unique<Responder>(os, dry_run);
    engine_.Subscribe([this](const Detection& detection) {
        responder_->Execute(planner_->Plan(detection));
    });
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

    std::print("[STAT] в модели процессов {}, файлов {}\n", model_.process_count(), model_.file_count());

    if (suppressed_ > 0) {
        std::print("[STAT] подавлено дубликатов {} (срок {} мс)\n", suppressed_, dedup_period_ms_);
    }
    if (responder_ != nullptr) {
        const auto& st = responder_->stats();
        std::print("[STAT] действий запланировано {}, повторов отброшено {}\n", st.planned, st.skipped_repeat);
        std::print("[STAT] завершено процессов {}, изолировано файлов {}, удалено файлов {}\n", st.killed, st.quarantined, st.deleted);
        if (st.own_events > 0) {
            std::print("[STAT] своих событий {}\n", st.own_events);
        }
    }
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

void Agent::PrintContextOnce() {
    if (context_printed_ || quiet_) {
        return;
    }
    PrintContext();
    context_printed_ = true;
}

void Agent::Subscribe(AlertSink sink) {
    FanOutSink all;
    
    all.Add([this](const Detection&) {
        PrintContextOnce();
    });
    
    all.Add(std::move(sink));

     engine_.Subscribe(DedupSink(std::move(all), dedup_period_ms_, &suppressed_));

    }
}
