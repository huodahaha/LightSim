#include "event_queue.h"

EventQueue::EventQueue() {}

void EventQueue::push_end(Event && event) {
    events.push(move(event));
}

Event EventQueue::pop_front() {
    assert(events.size() > 0);
    Event ret = move(events.front());
    events.pop();
    return move(ret);
}