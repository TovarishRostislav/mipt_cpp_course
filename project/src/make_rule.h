#pragma once

#include <memory>
#include <string>

#include "rules.h"

namespace nano_edr {

template <typename Rule, typename... Args>
std::unique_ptr<Rule> MakeRule(const std::string& id, Severity severity, Args... args) {
    return std::make_unique<Rule>(id, severity, args...);
}

} 