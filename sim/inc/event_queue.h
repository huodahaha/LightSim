#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <queue>
#include "event.h"

using namespace std;

class EventQueue {
private:
  // compariter of events needed
  priority_queue<Event *> _events;
public:
  EventQueue();
  // not copyable, not movable
  EventQueue(const EventQueue &) = delete;
  EventQueue & operator= (const EventQueue &) = delete;
  EventQueue(EventQueue &&) = delete;
  void push(Event *);
  Event * pop();
};

#endif
