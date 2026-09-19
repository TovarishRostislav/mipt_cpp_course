#include "event_list.h"

namespace nano_edr {

void ListPushBack(EventList* list, const Event* event) {
    EventNode* node = new EventNode;
    node->event = *event;          
    node->next  = nullptr;

    if (list->tail) {
        list->tail->next = node;
    } else {
        list->head = node;         
    }
    list->tail = node;
    ++list->size;


    if (list->capacity > 0 && list->size > list->capacity) {
        ListPopFront(list);
    }
}

void ListPopFront(EventList* list) {
    if (!list->head) return;     

    EventNode* old = list->head;
    list->head = old->next;

    if (!list->head) {
        list->tail = nullptr;     
    }

    delete old;
    --list->size;
}

void ListClear(EventList* list) {
    EventNode* cur = list->head;
    while (cur) {
        EventNode* next = cur->next;
        delete cur;
        cur = next;
    }
    list->head = nullptr;
    list->tail = nullptr;
    list->size = 0;

}

EventList::~EventList() {
    ListClear(this);
}

}  