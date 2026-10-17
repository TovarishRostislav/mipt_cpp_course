#include "conditions.h"

#include "text.h"
#include "fields.h"

namespace nano_edr{

bool FieldEquals::operator()(const Event& event) const{
    const std::string* p = FindField(event, key_);
    if(!p) return false;
    return EqualsIgnoreCase(*p, value_);
}

bool FieldContains::operator()(const Event& event) const{
    const std::string* p = FindField(event, key_);
    if(!p) return false;
    return ContainsIgnoreCase(*p, needle_);
}

bool FieldEndsWith::operator()(const Event& event) const{
    const std::string* p = FindField(event, key_);
    if(!p) return false;
    return EndsWithIgnoreCase(*p, suffix_);
}

bool FieldInList::operator()(const Event& event) const{
    if(values_.empty()) return false;

    const std::string* p = FindField(event, key_);
    if(!p) return false;

    for(const std::string& v : values_){
        if(EqualsIgnoreCase(*p, v)) return true;
    }
    return false;
}

Condition Condition::Equals(const std::string& key, const std::vector<std::string>& values){
    return Condition(Match::kEquals, key, values);
}

Condition Condition::Contains(const std::string& key, const std::vector<std::string>& values){
    return Condition(Match::kContains, key, values);
}

Condition Condition::EndsWith(const std::string& key, const std::vector<std::string>& values){
    return Condition(Match::kEndsWith, key, values);
}

bool Condition::operator()(const Event& event) const{
    if(values_.empty()) return false;

    const   std::string* p = FindField(event, key_);
    if(!p) return false;

    for(const std::string& v : values_ ){
        switch(match_){
            case Match::kEquals:
                if(EqualsIgnoreCase(*p, v)) return true;
                break;
            case Match::kContains:
                if(ContainsIgnoreCase(*p, v)) return true;
                break;
            case Match::kEndsWith:
                if(EndsWithIgnoreCase(*p, v)) return true;
                break;
        }
    }
    return false;
}

}