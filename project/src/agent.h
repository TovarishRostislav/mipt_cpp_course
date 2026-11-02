#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "event.h"
#include "event_list.h"
#include "event_source.h"   
#include "rule_engine.h"    
#include "rule.h"

namespace nano_edr {

class Agent : public IEventHandler {
public:
   Agent(std::size_t window_size, bool quiet);
   void HandleEvent(const Event& event) override;
   void OnSkipped() override;
   void OnBroken() override;
   void OnTick() override;

   void PrintSummary() const;

private:
   void PrintContext() const;

   EventList window_;
   bool quiet_;
   int64_t total_events_ = 0;
   std::map<std::string, int> type_counts_;
   const EventNode* ctx_ = nullptr;
   int64_t skipped_ = 0;
   int64_t broken_ = 0;
   RuleEngine engine_;
};

}
