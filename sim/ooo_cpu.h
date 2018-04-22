#ifndef CACHE_REPLACEMENT_OOO_CPU_H
#define CACHE_REPLACEMENT_OOO_CPU_H

#include "inc_all.h"
#include "event_engine.h"
#include "memory_hierarchy.h"
#include "trace_loader.h"

struct MemoryAccessInfo;
class MemoryUnit;
class CpuConnector;
class OoOCpuConnector;

struct CPUEventData : public EventDataBase {
  u32  opcode;
  u64  PC;
  u64  destination_registers[NUM_INSTR_DESTINATIONS];
  u64  source_registers[NUM_INSTR_SOURCES];
  u64  destination_memory[NUM_INSTR_DESTINATIONS];
  u64  source_memory[NUM_INSTR_SOURCES];
  bool ready;

  CPUEventData(const TraceFormat & t);
  CPUEventData(const CPUEventData & event_data) = default;
};

class CPU : public EventHandler {
 protected:
  u32 get_op_latency(const u32 opcode) const;
  bool has_destination_memory(CPUEventData *) const;
  bool has_source_memory(CPUEventData *) const;
  bool validate(EventType type) = 0;
  void proc(u64 tick, EventDataBase* data, EventType type) = 0;
 public:
  CPU(const string & tag) : EventHandler(tag) {}
};


class SequentialCPU : public CPU {
 private:
  const u8 _id;
  static const u8 _priority = 0;
  TraceFormat _current_trace;
  CpuConnector *_memory_connector;
 protected:
  bool validate(EventType type);
  void proc(u64 tick, EventDataBase* data, EventType type);
 public:
  SequentialCPU(const string &tag, u8 id, CpuConnector* _memory_connector);
  virtual ~SequentialCPU() {};
};


class OutOfOrderCPU : public CPU {
 private:
  struct RegisterAliasEntry {
    bool valid;
    u64 name;
  };

 private:
  // Todo: these should be configurable
  // defualt 4 way superscalar
  const size_t   _execution_list_limit = 4;
  const size_t   _issue_list_limit = 8;
  const size_t   _dispatch_list_limit = 8;
  const u32  _pipline_latency = 1;
  const u32  _pipline_bandwidth = 4;

  // this should be used with special care with event type
  static const u32  _zero_latency = 0;
  static const u8   _priority = 0;

  const u8 _id;

  u64 _register_name = 0;

  TraceFormat _current_trace;
  OoOCpuConnector * _memory_connector;

  RegisterAliasEntry _resgister_alias_table[128];

  list<CPUEventData*> _execution_list;
  list<CPUEventData*> _issue_list;
  list<CPUEventData*> _dispatch_list;


  inline bool execution_list_full() {
    return _execution_list.size() >= _execution_list_limit;
  }

  inline bool issue_list_full() {
    return _issue_list.size() >= _issue_list_limit;
  }

  inline bool dispatch_list_full() {
    return _dispatch_list.size() >= _dispatch_list_limit;
  }

  inline u64 new_register_name() {
    return _register_name++;
  }


  void rename_source_registers(CPUEventData *);
  void rename_destination_registers(CPUEventData *);

  bool ready_to_execute(CPUEventData *) const;
  void handle_WriteBack(EventEngine *, CPUEventData *);
  void handle_InstExecution(EventEngine *, CPUEventData *);
  void handle_InstIssue(EventEngine *, CPUEventData *);
  void handle_InstDispatch(EventEngine *, CPUEventData *);
  void handle_InstFetch(EventEngine *, CPUEventData *);



 protected:
  void proc(u64 tick, EventDataBase* data, EventType type);
  bool validate(EventType type);
 public:
  OutOfOrderCPU(const string &tag, u8 id, OoOCpuConnector* _memory_connector);
  virtual ~OutOfOrderCPU() {};
};





#endif // CACHE_REPLACEMENT_OOO_CPU_H
