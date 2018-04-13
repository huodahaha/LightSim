#ifndef EVENT_H
#define EVENT_H
#include "inc_all.h"

using namespace std;

class Event {
private:
    const u32 id;
    // Todo: make it more generic, and easy for member functions
    // function<void()> to_exec; // store callback function here?
public:
    // also store callback function
    Event(u32 id);
    // not copyable
    Event(const Event &) = delete;
    Event & operator= (const Event &) = delete;
    // but movable
    Event(Event &&) = default;
    virtual ~Event() = 0;
    u32 get_id() const;
    virtual void apply() = 0;
};
#endif
