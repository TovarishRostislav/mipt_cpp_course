#include "event_list.h"

namespace nano_edr {

void EventList::PushBack(const Event& event) {
    EventNode* node = new EventNode(event);
    
    if(tail_ == nullptr){
        head_ = node;
        tail_ = node;
        ++size_;
        return;
    }

    tail_->next = node;
    tail_ = node;
    ++size_;
    if(capacity_ != 0 && size_ > capacity_){
        PopFront();
    }
}

void EventList::PopFront() {
    if (!head_) return;  

    EventNode* old = head_;
    head_ = old->next;

    if (!head_) {
        tail_ = nullptr;     
    }

    delete old;
    --size_;
}

void EventList::Clear() {
    EventNode* cur = head_;
    while (cur) {
        EventNode* next = cur->next;
        delete cur;
        cur = next;
    }
    head_ = nullptr;
    tail_ = nullptr;
    size_ = 0;
}

EventList::~EventList() {
    Clear();
}

}  