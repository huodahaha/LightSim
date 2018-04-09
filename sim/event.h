#ifndef EVENT_H
#define EVENT_H

#include "inc_all.h"

using namespace std;

class Event {
private:
  //maybe useful for comparing the priority of the events
  enum class EventType {
    Memory_Event,
    CPU_Event
  };
  const u32 _id; // a unique id for every event
  u32 _cycle;    // scheduled cycle number
  // function<void()> _to_exec; // store callback function here
  EventType _type;
public:
  Event(u32 id);
  // not copyable
  Event(const Event &) = delete;
  Event & operator= (const Event &) = delete;
  // but movable
  Event(Event &&) = default;
  virtual ~Event() = 0;
  u32 get_id() const;
  virtual void execute() = 0;
};
#endif
