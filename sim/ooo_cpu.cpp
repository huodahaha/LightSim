#include "ooo_cpu.h"

CPUEventData::CPUEventData(const TraceFormat & t): opcode(t.opcode),
                                                   PC(t.pc) {
  memcpy(destination_memory, t.destination_memory, sizeof(destination_memory));
  memcpy(source_memory, t.source_memory, sizeof(source_memory));
}

u32 SequentialCPU::get_op_latency(const u32 opcode) const {
  //todo add more detialed latency
  (void)opcode;
  //reference http://www.agner.org/optimize/instruction_tables.pdf
  return 1;
}

SequentialCPU::SequentialCPU(const string &tag, u8 id,
                             CpuConnector *memory_connector)
    : EventHandler(tag), _id(id), _memory_connector(memory_connector) {
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

bool SequentialCPU::validate(EventType type) {
  return ((type == WriteBack) ||
          (type == InstExecution) ||
          (type == InstFetch));
}

void SequentialCPU::proc(u64 tick, EventDataBase* data, EventType type) {
  (void)tick;
  auto *event_data = (CPUEventData *) data;
  EventEngine *event_queue = EventEngineObj::get_instance();
  switch (type) {
    case WriteBack : {
      for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
        if (event_data->destination_memory[i] != 0) {
          MemoryAccessInfo writeback_info(event_data->PC,
                                        event_data->destination_memory[i]);
          _memory_connector->issue_memory_access(writeback_info, nullptr);
        }
      }
    } break;
    case InstExecution : {
      assert(event_data->ready);
      if (has_destination_memory(event_data)) {
        auto new_event_ddta = new CPUEventData(*event_data);
        Event *e = new Event(WriteBack, this, new_event_ddta);
        event_queue->register_after_now(e, get_op_latency(event_data->opcode),
                                        _priority);
      }
      Event *e = new Event(InstFetch, this, nullptr);
      event_queue->register_after_now(e, get_op_latency(event_data->opcode),
                                      _priority);
    } break;
    case InstFetch : {
      auto trace_loader = MultiTraceLoaderObj::get_instance();
      if(trace_loader->next_instruction(_id, _current_trace)) {
        CPUEventData *cpu_event_data = new CPUEventData(_current_trace);
        if (! has_source_memory(cpu_event_data)) {
          cpu_event_data->ready = true;
          Event *e = new Event(InstExecution, this, cpu_event_data);
          event_queue->register_after_now(e, 1, _priority);
        } else {
          for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
            if (cpu_event_data->source_memory[i] != 0) {
              MemoryAccessInfo read_info(cpu_event_data->PC,
                                         cpu_event_data->source_memory[i]);
              _memory_connector->issue_memory_access(read_info, cpu_event_data);
            }
          }
        }
      } else {
        SIMLOG(SIM_INFO, "CPU %d finished processing the trace file", _id);
        return;
      }
    } break;
    default:
      assert(0);
  }
}




//void OutOfOrderCPU::proc(u64 tick, EventDataBase* data, EventType type) {
//  (void)tick;
//  auto *cpu_event_data = (CPUEventData *) data;
//  EventEngine *event_queue = EventEngineObj::get_instance();
//  switch (type) {
//    case WriteBack : {
//      handle_WriteBack(event_queue, cpu_event_data);
//    } break;
//    case InstExecution : {
//      handle_InstExecution(event_queue, cpu_event_data);
//    } break;
//    case InstIssue : {
//      handle_InstIssue(event_queue, cpu_event_data);
//    } break;
//    case InstDispatch : {
//      handle_InstDispatch(event_queue, cpu_event_data);
//    } break;
//    default:
//      assert(0);
//  }
//}
//
//// Issue some MemoryOnAccess event & no need to wait for accessible
//// How to ignore the on arrive
//void OutOfOrderCPU::handle_WriteBack(EventEngine * event_queue,
//                                     CPUEventData * cpu_event_data) {
//  (void)event_queue;
//  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
//    if (cpu_event_data->destination_memory[i] != 0) {
//      MemoryAccessInfo acces_to_writeback(cpu_event_data->PC,
//                                          cpu_event_data->destination_memory[i]);
//      _memory_connector.issue_memory_access(acces_to_writeback);
//    }
//  }
//}
//
//
//// Get the opcode latency and Issue Writeback event if there are destination memory
//void OutOfOrderCPU::handle_InstExecution(EventEngine * event_queue,
//                                         CPUEventData * cpu_event_data) {
//#ifdef DEBUG
//  assert(! _execution_list.empty());
//  assert(cpu_event_data->ready);
//  assert(_execution_list.front() == cpu_event_data);
//#endif
//  if (has_destination_memory(cpu_event_data)) {
//    Event *e = new Event(WriteBack, this, cpu_event_data);
//    event_queue->register_after_now(e, get_operation_latency(cpu_event_data->opcode),
//                                    _priority);
//  }
//  Event *e = new Event(InstIssue, this, nullptr);
//  event_queue->register_after_now(e, get_operation_latency(cpu_event_data->opcode),
//                                  _priority);
//}
//
//// Get the
//void OutOfOrderCPU::handle_InstIssue(EventEngine * event_queue,
//                                     CPUEventData * cpu_event_data){
//  // check the issue queue of instructions and add execution event
//  if (! _issue_list.empty()) {
//    CPUEventData *to_exec_event = _issue_list.front();
//    _issue_list.pop_front();
//#ifdef DEBUG
//    assert(cpu_event_data == nullptr);
//    assert(to_exec_event->ready);
//    assert(_execution_list.empty());
//#endif
//    _execution_list.push_back(to_exec_event);
//    Event *e = new Event(InstExecution, this, to_exec_event);
//    event_queue->register_after_now(e, _issue_to_exec_latency, _priority);
//  } else {
//    // no instruction ready, read from dispatch list
//    Event *e = new Event(InstDispatch, this, nullptr);
//    event_queue->register_after_now(e, _zero_latency, _priority);
//  }
//}
//
//
//void OutOfOrderCPU::handle_InstDispatch(EventEngine * event_queue,
//                                        CPUEventData * cpu_event_data) {
//  if (! _dispatch_list.empty()) {
//
//  }
//}