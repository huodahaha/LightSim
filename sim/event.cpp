#include "event.h"

Event::Event(u32 id): _id(id) {}

u32 Event::get_id() const { return _id;}