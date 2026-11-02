#include "conditions.h"

#include "text.h"
#include "fields.h"

namespace nano_edr{

bool FieldEquals::Matches(const Event& event) const{
    const std::string* p = FindField(event, key_);
    if(!p) return false;
    return EqualsIgnoreCase(*p, value_);
}

bool FieldContains::Matches(const Event& event) const{
    const std::string* p = FindField(event, key_);
    if(!p) return false;
    return ContainsIgnoreCase(*p, needle_);
}

bool FieldEndsWith::Matches(const Event& event) const{
    const std::string* p = FindField(event, key_);
    if(!p) return false;
    return EndsWithIgnoreCase(*p, suffix_);
}

bool FieldInList::Matches(const Event& event) const{
    if(values_.empty()) return false;

    const std::string* p = FindField(event, key_);
    if(!p) return false;

    for(const std::string& v : values_){
        if(EqualsIgnoreCase(*p, v)) return true;
    }
    return false;
}

bool EventTypeIs::Matches(const Event& event) const{
    for(EventType t :  types_){
        if(event.event_type() == t) return true;
    }
    return false;
}

void AllOf::Add(std::unique_ptr<ICondition> condition){
    parts_.push_back(std::move(condition));
}

bool AllOf::Matches(const Event& event) const{
    for(const auto& c : parts_){
        if(!c->Matches(event)) return false;
    }
    return true;
}

void AnyOf::Add(std::unique_ptr<ICondition> condition){
    parts_.push_back(std::move(condition));
}

bool AnyOf::Matches(const Event& event) const {
    for(const auto& c : parts_){
        if(c->Matches(event)) return true;
    }
    return false;
}
}