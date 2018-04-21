#ifndef CACHE_REPLACEMENT_OOO_CPU_H
#define CACHE_REPLACEMENT_OOO_CPU_H

#include "inc_all.h"
#include "event_engine.h"
#include "memory_hierarchy.h"
#include "trace_loader.h"

// on_memory_acess
// 重复的address会被
// poll




struct CPUEventData : public EventDataBase {
  u32  opcode;
  u64  PC;
  u64  destination_memory[NUM_INSTR_DESTINATIONS];
  u64  source_memory[NUM_INSTR_SOURCES];
  bool ready;
  CPUEventData(const TraceFormat &);
};


class SequentialCPU : public EventHandler {
 private:
  const size_t             _id;
  const MultiTraceLoader * _multi_trace_loader;
  TraceFormat              _current_trace;
  CpuConnector             _memomry_connecter;

  list<u64>                 _pending_refs;

  list<CPUEventData*>       _execution_list;
  list<CPUEventData*>       _issue_list;
  list<CPUEventData*>       _dispatch_list;


  size_t get_operation_latency(const u32 opcode) const;
  bool has_destination_memory(CPUEventData *) const;
  bool has_source_memory(CPUEventData *) const;

  const u8   _priority = 1;
  const u8   _execution_list_limit = 1;
  const u8   _issue_list_limit = 8;
  const u8   _dispatch_list_limit = 16;

  const u32  _issue_to_exec_latency = 1;
  const u32  _dispath_to_issue_latency = 1;

  inline bool execution_list_full() {
    return _execution_list.size() >= _execution_list_limit;
  }

  inline bool issue_list_full() {
    return _issue_list.size() >= _issue_list_limit;
  }

  inline bool dispatch_list_full() {
    return _dispatch_list.size() >= _dispatch_list_limit;
  }

 protected:
  void proc(u64 tick, EventDataBase* data, EventType type);

 public:
  // also initialize _memomry_connecter
//  SequentialCPU(size_t id, MultiTraceLoaderObj multi_trace_loader):
//      _id(id), _multi_trace_loader(multi_trace_loader.get_instance()) {}
};



//class OutOfOrderCPU : public MemoryUnit {
//
//};

#endif // CACHE_REPLACEMENT_OOO_CPU_H
