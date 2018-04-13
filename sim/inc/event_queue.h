#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <queue>
#include "event.h

using namespace std;

class EventQueue {
private:
    queue<Event> events;
public:
    EventQueue();
    // not copyable, not movable
    EventQueue(const EventQueue &) = delete;
    EventQueue & operator= (const EventQueue &) = delete;
    EventQueue(EventQueue &&) = delete;

    void push_end(Event &&);
    Event pop_front();
};

#endif
