#include "event.h"

Event::Event(u32 id): id(id) {}

u32 Event::get_id() const { return id;}