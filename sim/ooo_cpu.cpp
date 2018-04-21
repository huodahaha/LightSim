#include "ooo_cpu.h"


size_t SequentialCPU::get_operation_latency(const u32 opcode) const {
  //todo get corresponding latency
  return 1;
}

bool SequentialCPU::has_destination_memory(CPUEventData * event_data) const {
  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    if (event_data->destination_memory[i] != 0) return true;
  }
  return false;
}

bool SequentialCPU::has_source_memory(CPUEventData * event_data) const {
  for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
    if (event_data->source_memory[i] != 0) return true;
  }
  return false;
}

void SequentialCPU::proc(u64 tick, EventDataBase* data, EventType type) {
  (void)tick;
  auto *cpu_event_data = (CPUEventData *) data;
  EventEngine *event_queue = EventEngineObj::get_instance();
  switch (type) {
    // Issue some MemoryOnAccess event & no need to wait for accessible
    // How to ignore the on arrive
    case WriteBack : {
#ifdef DEBUG
      assert(! _execution_list.empty());
      assert(_execution_list.front() == cpu_event_data);
#endif
      _execution_list.pop_front();
      for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
        if (cpu_event_data->destination_memory[i] != 0) {
          MemoryAccessInfo acces_to_writeback(cpu_event_data->PC,
                                              cpu_event_data->destination_memory[i]);
              _memomry_connecter.issue_memory_access(acces_to_writeback);
        }
      }
    } break;
    // Get the opcode latency and Issue Writeback event if there are destination memory
    case InstExecution : {
      if (_execution_list.empty()) {
#ifdef DEBUG

#endif
        Event *e = new Event(InstIssue, this, nullptr);
        event_queue->register_after_now(e, _issue_to_exec_latency, _priority);
      }
      if (has_destination_memory(cpu_event_data)) {
        Event *e = new Event(WriteBack, this, cpu_event_data);
        event_queue->register_after_now(e, get_operation_latency(cpu_event_data->opcode),
                                        _priority);
      } else {
        _execution_list.pop_front();
      }
      Event *e = new Event(InstIssue, this, nullptr);
      event_queue->register_after_now(e, get_operation_latency(cpu_event_data->opcode),
                                      _priority);
    } break;
    case InstIssue : {
      // check the issue queue of instructions and add execution event
      if (! _issue_list.empty()) {
        CPUEventData *to_exec_event = _issue_list.front();
        _issue_list.pop_front();
#ifdef DEBUG
        assert(to_exec_event->ready);
        assert(_execution_list.empty());
#endif
        _execution_list.push_back(to_exec_event);
        Event *e = new Event(InstIssue, this, to_exec_event);
        event_queue->register_after_now(e, _issue_to_exec_latency, _priority);
      } else {
        // no instruction ready, read from
        Event *e = new Event(InstDispatch, this, nullptr);
        event_queue->register_after_now(e, 0, _priority);
      }


    } break;
    // If the instruction list is not full read new instruction
    case InstDispatch : {
      if (has_source_memory(cpu_event_data)) {
        // check the pending status of the source memory and create InstExecution event
      } else {}
    } break;
    case InstFetch: {

    } break;
  }

}

