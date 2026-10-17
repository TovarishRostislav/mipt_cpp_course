#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "event.h"
#include "event_list.h"
#include "os.h"
#include "rule.h"

namespace nano_edr {

class Agent {
 public:
    Agent(std::size_t window_size, bool quiet);
    void HandleEvent(const Event& event);
    void PrintSummary() const;

    static void Trampoline(const os_event* ev, void* ctx) noexcept;

 private:
    void PrintContext() const;

    EventList window_;
    bool quiet_;
    long long total_events_ = 0;
    std::map<std::string, int> type_counts_;
    std::vector<MatchRule> rules_;
    const EventNode* ctx_ = nullptr;
};

}
