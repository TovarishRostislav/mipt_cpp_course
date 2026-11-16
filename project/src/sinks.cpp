#include <stdexcept>
#include <string>
#include <iostream>

#include "alert_sink.h"
#include "dedup_sink.h"
#include "detection.h"

namespace nano_edr {

void ConsoleSink::operator()(const Detection& detection) const {
    std::cout << ToString(detection) << '\n';
}


FileSink::FileSink(const std::string& path) : out_(path) {
    if (!out_.is_open()) {
        throw std::runtime_error("не удалось открыть файл: " + path);
    }
}

void FileSink::operator()(const Detection& detection) {
    out_ << ToString(detection) << '\n';
}

void CountingSink::operator()(const Detection& detection) const {
    if(counter_ != nullptr){
        counter_->total++;
        counter_->seen.push_back(detection);
    }
}
void FanOutSink::Add(AlertSink sink) {
    sinks_.push_back(std::move(sink));
}

void FanOutSink::operator()(const Detection& detection) const {
    for (const auto& sink : sinks_) {
        sink(detection);
    }
}

DedupSink::DedupSink(AlertSink inner, uint64_t period_ms, std::size_t* suppressed)
    : inner_(std::move(inner)), 
      period_ms_(period_ms), 
      suppressed_(suppressed) {}

void DedupSink::operator()(const Detection& detection) const {
    if (period_ms_ == 0) {
        inner_(detection);
        return;
    }

    for (auto& record : records_) {
        if (record.rule == detection.rule && record.pid == detection.pid) {
            uint64_t diff = detection.ts.ms - record.first_seen.ms;
            
            if (diff <= period_ms_) {
                if (suppressed_ != nullptr) {
                    ++(*suppressed_);
                }
                return;
            } else {
                record.first_seen = detection.ts; 
                inner_(detection);
                return;
            }
        }
    }

    records_.push_back(Record{detection.rule, detection.pid, detection.ts});
    inner_(detection);
}

} 
