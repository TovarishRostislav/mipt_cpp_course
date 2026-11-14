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
#include "entity_model.h"
#include "dedup_sink.h"

namespace nano_edr {

struct WindowEntry {
   Timestamp ts;
   std::string type;
   std::string pid;
};

class Agent : public IEventHandler {
public:
   static constexpr size_t kWindowCap = 256;  
   explicit Agent(bool quiet, uint64_t dedup_period_ms);
   void HandleEvent(const Event& event) override;
   void OnSkipped() override;
   void OnBroken() override;  
   void OnTick() override;
   void Subscribe(AlertSink sink);
   void PrintSummary() const;

private:
   bool quiet_;
   uint64_t dedup_period_ms_ = 0;
   EntityModel model_;
   RuleEngine engine_;
   RingBuffer<WindowEntry, kWindowCap> window_;
   void PrintContext() const;
   void PrintContextOnce();
   bool context_printed_ = false;
   int64_t total_events_ = 0;
   std::map<std::string, int> type_counts_;
   int64_t skipped_ = 0;
   int64_t broken_ = 0;
   size_t suppressed_ = 0;
};
}