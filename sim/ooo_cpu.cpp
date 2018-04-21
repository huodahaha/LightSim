#include "ooo_cpu.h"

//WriteBack,
//InstExecution,
//InstIssue,
//InstDispatch,
//InstFetch

void SequentialCPU::proc(u64 tick, EventDataBase* data, EventType type) {
  (void)tick;
  CPUEventData *cpu_data = (CPUEventData *) data;
  EventEngine *evnet_queue = EventEngineObj::get_instance();

  switch (type) {
    // Issue some MemoryOnAccess event & no need to wait for accessible
    case WriteBack : {
    } break;
    // Get the opcode latency and Issue Writeback event if there are destination memory
    case InstExecution : {} break;
    // check ready state of all the operand of instruction and issue InstExecution
    case InstIssue : {} break;
    // If the instruction list is not full read new instruction
    case InstFetch : {} break;
  }

}

