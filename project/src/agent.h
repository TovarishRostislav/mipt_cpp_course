#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "ring_buffer.h"
#include "event_source.h"   
#include "rule_engine.h"    
#include "rule.h"

namespace nano_edr {

struct WindowEntry {
   Timestamp ts;
   std::string type;
   std::string pid;
};

class Agent : public IEventHandler {
public:
   static constexpr size_t kWindowCap = 256;  
   explicit Agent(bool quiet);
   void HandleEvent(const Event& event) override;
   void OnSkipped() override;
   void OnBroken() override;
   void OnTick() override;

   void PrintSummary() const;

private:
   void PrintContext() const;

   RingBuffer<WindowEntry, kWindowCap> window_;
   bool quiet_;
   int64_t total_events_ = 0;
   std::map<std::string, int> type_counts_;
   int64_t skipped_ = 0;
   int64_t broken_ = 0;
   RuleEngine engine_;
};

}