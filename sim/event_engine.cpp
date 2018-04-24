#include "event_engine.h"

static const u8 TICK_FACTOR = 10;
static const u8 TYPE_FACTOR = 6;

static string type_name[TypeCount] = {
  "Reserved",
  "MemoryOnAccess",
  "MemoryOnArrive",
  
  "ReorderBufferRetire",
  "InstExecution",
  "InstIssue",
  "InstDispatch",
  "InstFetch",
  "PidCensus",
};

string event_type_to_string(EventType type) {
  return type_name[type];
}

void EventHandler::proc_event(u64 tick, Event *e) {
  assert(validate(e->type));
  proc(tick, e->callbackdata, e->type);
}

void Event::execute(u64 tick) {
  handler->proc_event(tick, this);
}

void EventEngine::register_after_now(Event* e, u32 ticks, u32 priority) {
  assert(priority < (1 << TYPE_FACTOR));

  s64 pv = ((_tick + ticks) << TICK_FACTOR);
  pv += 1 << TICK_FACTOR;
  pv -= e->type << TYPE_FACTOR;
  pv -= priority;
  _queue.insert({pv, e});
}

s32 EventEngine::loop() {
  if (_queue.size() == 0) {
    return 0;
  }

  auto iter = _queue.begin();
  auto e = iter->second;
  _tick = ((iter->first) >> TICK_FACTOR);
  _queue.erase(iter);
  e->execute(_tick);
  delete e;
  return 1;
}
