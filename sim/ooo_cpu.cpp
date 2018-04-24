#include "ooo_cpu.h"

// CPUEventData::CPUEventData(const TraceFormat & t): opcode(t.opcode),
CPUEventData::CPUEventData(const TraceFormat & t): PC(t.pc) {
  //dummy opcode
  opcode = 0;
  for (u8 i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    dreg_rename[i] = 0;
  }
  for (u8 i = 0; i < NUM_INSTR_SOURCES; i++) {
    sreg_rename[i] = 0;
    if (t.source_registers[i] == 0) sreg_ready[i] = true;
    else sreg_ready[i] = false;
  }
  memcpy(destination_registers, t.destination_registers, sizeof(destination_registers));
  memcpy(source_registers, t.source_registers, sizeof(source_registers));
  memcpy(destination_memory, t.destination_memory, sizeof(destination_memory));
  memcpy(source_memory, t.source_memory, sizeof(source_memory));
}

u32 CPU::get_op_latency(const u32 opcode = 0) const {
  //todo add more detialed latency
  (void)opcode;
  //reference http://www.agner.org/optimize/instruction_tables.pdf
  return 1;
}

SequentialCPU::SequentialCPU(const string &tag, u8 id,
                             CpuConnector *memory_connector)
    : CPU(tag), _id(id), _memory_connector(memory_connector) {
}


bool CPU::has_destination_memory(CPUEventData * event_data) const {
  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    if (event_data->destination_memory[i] != 0) return true;
  }
  return false;
}

bool CPU::has_source_memory(CPUEventData * event_data) const {
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
          MemoryAccessInfo writeback_info(event_data->destination_memory[i],
                                          event_data->PC, _id);
          _memory_connector->issue_memory_access(writeback_info, nullptr);
        }
      }
    } break;
    case InstExecution : {
      assert(event_data->memory_ready);
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
          cpu_event_data->memory_ready = true;
          Event *e = new Event(InstExecution, this, cpu_event_data);
          event_queue->register_after_now(e, 1, _priority);
        } else {
          for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
            if (cpu_event_data->source_memory[i] != 0) {
              MemoryAccessInfo read_info(cpu_event_data->source_memory[i],
                                         cpu_event_data->PC, _id);
              _memory_connector->issue_memory_access(read_info, cpu_event_data);
            }
          }
        }
      } else {
        auto census_taker = CensusTakerObj::get_instance();
        census_taker->shutdown();
        SIMLOG(SIM_INFO, "CPU %d finished processing the trace file\n", _id);
        return;
      }
    } break;
    default:
      assert(0);
  }
}

/*
OutOfOrderCPU::OutOfOrderCPU(const string &tag, u8 id,
                             OoOCpuConnector *memory_connector)
    : CPU(tag), _id(id), _memory_connector(memory_connector) {
}

bool OutOfOrderCPU::ready_to_execute(CPUEventData * event_data) const {
  if (!event_data->memory_ready) return false;
  // check the source register available status
  for (u64 sreg : event_data->source_registers) {

  }
}

void OutOfOrderCPU::rename_source_registers(CPUEventData * event_data) {
  for (u8 i = 0; i < NUM_INSTR_SOURCES; i++) {
    u8 sreg = event_data->source_registers[i];
    if (sreg == 0) continue; // zero means not used
    // not need to rename source register if it's memory_ready
    // set the private memory_ready bit
    if (_resgister_alias_table[sreg].ready) {
      event_data->sreg_ready[i] = true;
    }
    else {
      // we need to give it a new name as we don't know when it will be memory_ready
      // the name is also a name of destination register of previous instruction
      event_data->sreg_rename[i] = _resgister_alias_table[sreg].name;
    }
  }
}

void OutOfOrderCPU::rename_destination_registers(CPUEventData * event_data) {
  for (u8 i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    u8 dreg = event_data->destination_registers[i];
    if (dreg == 0) continue;
    // generate a new name for destination
    _resgister_alias_table[dreg].name = new_register_name();
    // clear the memory_ready bit
    // we must be very careful about the timing of event because of this
    _resgister_alias_table[dreg].ready = false;
  }
}

void OutOfOrderCPU::check_ready_after_execu(CPUEventData * event_data) {
  //this function does not take clock cycle
  //because of the CDB
  for (u8 i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    u8 dreg = event_data->destination_registers[i];
    if (dreg == 0) continue;
    u64 name = event_data->dreg_rename[i];
    if (_resgister_alias_table[dreg].name == name)
      _resgister_alias_table[dreg].ready = true;
    for (CPUEventData *waiting_event_data : _issue_list) {
      set_ready_by_name(name, waiting_event_data);
    }
  }
}

void OutOfOrderCPU::set_ready_by_name(u64 name, CPUEventData *waiting_event_data) {
  for (u8 i = 0; i < NUM_INSTR_SOURCES; i++) {
    if (waiting_event_data->sreg_rename[i] == name) {
      waiting_event_data->sreg_ready[i] = true;
    }
  }
}

bool OutOfOrderCPU::check_entry_ready(CPUEventData * event_data) const {
  bool ret = true;
  for (u8 i = 0; i < NUM_INSTR_SOURCES; i++) {
    ret = ret & event_data->sreg_ready[i];
  }
  ret = ret & event_data->memory_ready;
  return ret;
}

CPUEventData * OutOfOrderCPU::find_ready_entry() {
  CPUEventData * ret = nullptr;
  for (auto itr = _issue_list.begin(); itr != _issue_list.end(); itr++) {
    if (check_entry_ready(*itr)) {
      ret = *itr;
      _issue_list.erase(itr);
      return ret;
    }
  }
  return nullptr;
}

bool OutOfOrderCPU::validate(EventType type) {
  return ((type == WriteBack) ||
      (type == InstExecution) ||
      (type == InstIssue)     ||
      (type == InstDispatch)  ||
      (type == InstFetch)     ||
      (type == MemoryOnAccessCPU));
}

void OutOfOrderCPU::proc(u64 tick, EventDataBase* data, EventType type) {
  (void)tick;
  auto *cpu_event_data = (CPUEventData *) data;
  EventEngine *event_queue = EventEngineObj::get_instance();
  switch (type) {
    case MemoryOnAccessCPU : {
      //Todo handle MemoryOnAccessCPU
    }  break;
    case WriteBack : {
      handle_WriteBack(event_queue, cpu_event_data);
    } break;
    case InstExecution : {
      handle_InstExecution(event_queue, cpu_event_data);
    } break;
    case InstIssue : {
      handle_InstIssue(event_queue, cpu_event_data);
    } break;
    case InstDispatch : {
      handle_InstDispatch(event_queue, cpu_event_data);
    } break;
    case InstFetch : {
      handle_InstFetch(event_queue, cpu_event_data);
    } break;
    default:
      assert(0);
  }
}

// Issue some MemoryOnAccess event & no need to wait for accessible
// How to ignore the on arrive??
void OutOfOrderCPU::handle_WriteBack(EventEngine * event_queue,
                                     CPUEventData * cpu_event_data) {
  for (auto itr = _execution_list.begin();
       itr != _execution_list.end(); itr++) {
    if (*itr == cpu_event_data) {
      _execution_list.erase(itr);
      break;
    }
  }
  Event *e = new Event(InstIssue, this, nullptr);
  event_queue->register_after_now(e, _pipline_latency, _priority);
  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    if (cpu_event_data->destination_memory[i] != 0) {
      MemoryAccessInfo acces_to_writeback(cpu_event_data->PC,
                                          cpu_event_data->destination_memory[i]);
      //Todo
//      _memory_connector->issue_memory_access(acces_to_writeback);
    }
  }
}


// Get the opcode latency and Issue Writeback
void OutOfOrderCPU::handle_InstExecution(EventEngine * event_queue,
                                         CPUEventData * event_data) {
  assert(event_data == nullptr);
  for (CPUEventData * cpu_event_data : _execution_list) {
    Event *e = new Event(WriteBack, this, cpu_event_data);
    event_queue->register_after_now(e, get_op_latency(cpu_event_data->opcode),
                                    _priority);
  }
}

// this event should be the main source of other cpu event
void OutOfOrderCPU::handle_InstIssue(EventEngine * event_queue,
                                     CPUEventData * event_data){
  assert(event_data == nullptr);
  u8 count = 0;
  bool issued_flag = false;
  while (count++ < _pipline_bandwidth && ! execution_list_full()) {
    // check the issue queue of instructions and add execution event
    for (auto itr = _issue_list.begin(); itr != _issue_list.end(); itr++) {
      if (ready_to_execute(*itr)) {
        issued_flag = true;
        _execution_list.push_back(*itr);
        _issue_list.erase(itr);
        break;
      }
    }
  }
  if (issued_flag) {
    Event *e = new Event(InstExecution, this, nullptr);
    event_queue->register_after_now(e, _pipline_latency, _priority);
  }
}


void OutOfOrderCPU::handle_InstDispatch(EventEngine * event_queue,
                                        CPUEventData * event_data) {
  assert(event_data == nullptr);
  u8 count = 0;
  bool dispathed_flag = false;
  while (count++ < _pipline_bandwidth && ! issue_list_full()) {
    dispathed_flag = true;
    if (_dispatch_list.empty()) break;
    // this must be in order
    CPUEventData *cpu_event_data = _dispatch_list.front();
    _dispatch_list.pop_front();
    //rename the register based on the memory_ready and valid bit
    rename_source_registers(cpu_event_data);
    rename_destination_registers(cpu_event_data);
    if (!has_source_memory(cpu_event_data)) {
      cpu_event_data->memory_ready = true;
    } else {
      for (u64 source_memory : cpu_event_data->source_memory) {
        if (source_memory == 0) continue;
        //Todo call the OoOCpuConnector
        //      _memory_connector->issue_memory_access();
      }
      _issue_list.push_back(cpu_event_data);
    }
  }
  if (dispathed_flag) {
    Event *e = new Event(InstIssue, this, nullptr);
    event_queue->register_after_now(e, _pipline_latency, _priority);
  }
}


void OutOfOrderCPU::handle_InstFetch(EventEngine * event_queue,
                                        CPUEventData * event_data) {
  assert(event_data == nullptr);
  u8 count = 0;
  auto trace_loader = MultiTraceLoaderObj::get_instance();
  while (count++ < _pipline_bandwidth && !dispatch_list_full()) {
    if (!trace_loader->next_instruction(_id, _current_trace)) {
      return;
    }
    CPUEventData *cpu_event_data = new CPUEventData(_current_trace);
    _dispatch_list.push_back(cpu_event_data);
  }

  Event *e = new Event(InstDispatch, this, nullptr);
  event_queue->register_after_now(e, _pipline_latency, _priority);

  if (!dispatch_list_full()) {
    Event *e = new Event(InstFetch, this, nullptr);
    event_queue->register_after_now(e, _pipline_latency, _priority);
  }
}
*/
