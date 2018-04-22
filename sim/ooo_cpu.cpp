#include "ooo_cpu.h"

CPUEventData::CPUEventData(const TraceFormat & t): opcode(t.opcode),
                                                   PC(t.pc) {
  for (u8 i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    destination_registers[i] = t.destination_registers[i];
  }
  for (u8 i = 0; i < NUM_INSTR_SOURCES; i++) {
    source_registers[i] = t.source_registers[i];
  }
  memcpy(destination_memory, t.destination_memory, sizeof(destination_memory));
  memcpy(source_memory, t.source_memory, sizeof(source_memory));
}

u32 CPU::get_op_latency(const u32 opcode) const {
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


OutOfOrderCPU::OutOfOrderCPU(const string &tag, u8 id,
                             OoOCpuConnector *memory_connector)
    : CPU(tag), _id(id), _memory_connector(memory_connector) {
}

bool OutOfOrderCPU::ready_to_execute(CPUEventData * event_data) const {
  if (!event_data->ready) return false;
  // check the source register available status
  for (u64 sreg : event_data->source_registers) {

  }
}

bool OutOfOrderCPU::validate(EventType type) {
  return ((type == WriteBack) ||
      (type == InstExecution) ||
      (type == InstIssue)     ||
      (type == InstDispatch)  ||
      (type == InstFetch));
}

void OutOfOrderCPU::proc(u64 tick, EventDataBase* data, EventType type) {
  (void)tick;
  auto *cpu_event_data = (CPUEventData *) data;
  EventEngine *event_queue = EventEngineObj::get_instance();
  switch (type) {
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
// How to ignore the on arrive
void OutOfOrderCPU::handle_WriteBack(EventEngine * event_queue,
                                     CPUEventData * cpu_event_data) {
  (void)event_queue;
  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    if (cpu_event_data->destination_memory[i] != 0) {
      MemoryAccessInfo acces_to_writeback(cpu_event_data->PC,
                                          cpu_event_data->destination_memory[i]);
      _memory_connector.issue_memory_access(acces_to_writeback);
    }
  }
}


// Get the opcode latency and Issue Writeback event if there are destination memory
void OutOfOrderCPU::handle_InstExecution(EventEngine * event_queue,
                                         CPUEventData * cpu_event_data) {
#ifdef DEBUG
  assert(! _execution_list.empty());
  assert(cpu_event_data->ready);
  assert(_execution_list.front() == cpu_event_data);
#endif
  if (has_destination_memory(cpu_event_data)) {
    Event *e = new Event(WriteBack, this, cpu_event_data);
    event_queue->register_after_now(e, get_op_latency(cpu_event_data->opcode),
                                    _priority);
  }
  Event *e = new Event(InstIssue, this, nullptr);
  event_queue->register_after_now(e, get_op_latency(cpu_event_data->opcode),
                                  _priority);
}

void OutOfOrderCPU::handle_InstIssue(EventEngine * event_queue,
                                     CPUEventData * event_data){
  assert(event_data == nullptr);
  u8 count = 0;
  while (count++ < _pipline_bandwidth && ! execution_list_full()) {
    // check the issue queue of instructions and add execution event
    for (auto itr = _issue_list.begin(); itr != _issue_list.end(); itr++) {
      if (ready_to_execute(*itr)) {
        //Todo make a new event
        _issue_list.erase(itr);
      }
    }
  }
}


void OutOfOrderCPU::handle_InstDispatch(EventEngine * event_queue,
                                        CPUEventData * event_data) {
  assert(event_data == nullptr);
  u8 count = 0;
  while (count++ < _pipline_bandwidth && ! issue_list_full()) {
    if (_dispatch_list.empty()) break;
    CPUEventData *cpu_event_data = _dispatch_list.front();
    _dispatch_list.pop_front();
    //rename the register based on the ready and valid bit
    rename_source_registers(cpu_event_data);
    rename_destination_registers(cpu_event_data);
//    CPUEventData renamed_event = new CPUEventData(cpu_event_data)
    for (u64 source_memory : cpu_event_data->source_memory) {
      if (source_memory == 0) continue;
      //Todo call the OoOCpuConnector
      _memory_connector->issue_memory_access();
    }
    //Todo Make it an event
    _issue_list.push_back(cpu_event_data);
  }
}


void OutOfOrderCPU::handle_InstFetch(EventEngine * event_queue,
                                        CPUEventData * event_data) {
  assert(event_data == nullptr);
  u8 count = 0;
  auto trace_loader = MultiTraceLoaderObj::get_instance();
  while (count++ < _pipline_bandwidth && ! dispatch_list_full()) {
    if (!trace_loader->next_instruction(_id, _current_trace)) {
      break;
    }
    CPUEventData *cpu_event_data = new CPUEventData(_current_trace);
    //Todo Make it an event
    _dispatch_list.push_back(cpu_event_data);
  }
}


OoOCpuConnector::OoOCpuConnector(const string &tag, u8 id): MemoryUnit(tag, 0, 0),
                                                      _waiting_event_data(nullptr) {
  _cpu_ptr = new OoOCpuConnector(tag, id, this);
}

OoOCpuConnector::~OoOCpuConnector() {
  delete _cpu_ptr;
}

bool OoOCpuConnector::try_access_memory(const MemoryAccessInfo &info) {
  (void)info;
  return false;
}

void OoOCpuConnector::on_memory_arrive(const MemoryAccessInfo &info) {
  //todo make a new kind of event that tells the cpu the memory arrive
  if (! _waiting_event_data) return;
  auto iter = _pending_refs.find(info.addr);
  if (iter != _pending_refs.end()) {
    _pending_refs.erase(iter);
  }
  if (_pending_refs.empty()) {
    auto evnet_queue = EventEngineObj::get_instance();
    Event *e = new Event(InstExecution, _cpu_ptr, _waiting_event_data);
    evnet_queue->register_after_now(e, 1, get_priority());
    _waiting_event_data->ready = true;
    _waiting_event_data = nullptr;
  }
}


void OoOCpuConnector::issue_memory_access(const MemoryAccessInfo &info,
                                       CPUEventData *event_data) {
  auto evnet_queue = EventEngineObj::get_instance();
  MemoryEventData *d = new MemoryEventData(info);
  Event *e = new Event(MemoryOnAccess, this, d);
  evnet_queue->register_after_now(e, 0, get_priority());
  if (event_data) {
    _waiting_event_data = event_data;
    _pending_refs.insert(info.addr);
  }
}

void OoOCpuConnector::start() {
  auto event_queue = EventEngineObj::get_instance();
  Event *e = new Event(InstFetch, _cpu_ptr, nullptr);
  event_queue->register_after_now(e, 0, _priority);
}