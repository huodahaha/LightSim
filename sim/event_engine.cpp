#include "event_engine.h"

static const u8 TICK_FACTOR = 10;
static const u8 TYPE_FACTOR = 6;

void EventHandler::proc_event(Event *e) {
  assert(e->type == get_type());
  proc(e->callbackdata);
}

void Event::execute() {
  handler->proc_event(this);
}

void EventEngine::register_after_now(Event* e, u32 ticks, u32 priority) {
  assert(priority < (1 << TYPE_FACTOR));

  s64 pv = -1 * ((_tick + ticks) << TICK_FACTOR);
  pv += e->type << TYPE_FACTOR;
  pv += priority;
  _queue.insert({pv, e});
}

int EventEngine::loop() {
  if (_queue.size() == 0) {
    return 0;
  }

  auto iter = _queue.begin();
  auto e = iter->second;
  _tick = -1 * ((iter->first) >> TICK_FACTOR);
  _queue.erase(iter);
  e->execute();
  delete e;
  return 1;
}
