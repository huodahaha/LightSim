//
// Created by Ma Huri on 4/13/18.
//

#include "trace_loader.h"

// TraceFormat::TraceFormat() : pc(0), opcode(0), thread_id(0),
TraceFormat::TraceFormat() : pc(0), 
                             is_branch(0), branch_taken(0) {
  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    destination_registers[i] = 0;
    destination_memory[i] = 0;
  }
  for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
    source_registers[i] = 0;
    source_memory[i] = 0;
  }
}

TraceLoader::TraceLoader(const string &filename) {
  char gzip_command[512];
  sprintf(gzip_command, "gunzip -c %s", filename.c_str());
  _trace_file = popen(gzip_command, "r");
  if (_trace_file == nullptr) {
    SIMLOG(SIM_ERROR, "\nUnable to read the trace file %s, exiting.\n",
           filename.c_str());
    exit(-1);
  }
}

TraceLoader::~TraceLoader() { pclose(_trace_file); }

void TraceLoader::set_read_bound(s64 bound) {
  _bound = bound;
}

size_t TraceLoader::next_instruction(TraceFormat &trace) {
  if (_bound == -1 || _count++ < _bound) {
    return fread(&trace, sizeof(TraceFormat), 1, _trace_file);
  }
  else {
    return 0;
  }
}

MultiTraceLoader::~MultiTraceLoader() {
  for (TraceLoader *trace_loader : _trace_loaders) {
    delete trace_loader;
  }
}

void MultiTraceLoader::adding_trace(const string &filename) {
  auto trace_loader = new TraceLoader(filename);
  trace_loader->set_read_bound(_bound);
  _trace_loaders.push_back(trace_loader);
}

size_t MultiTraceLoader::get_trace_num() const {
  return _trace_loaders.size();
}

s32 MultiTraceLoader::assign_trace() {
  if (_cur_assigned == get_trace_num())
    return -1;
  else
    return _cur_assigned++;
}

size_t MultiTraceLoader::next_instruction(u32 trace_id, TraceFormat &trace) {
  assert(trace_id < this->get_trace_num());
  //64 bit VM range
  //0x000'00000000 through 0x7FF'FFFFFFFF
  // this is a magic number for resolving VM address conflict
  const u64 shift =  0xFFFFFFFFF;
  const u64 range =  0x7FFFFFFFFFF;
  size_t ret = _trace_loaders[trace_id]->next_instruction(trace);
  for (u32 i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    if (trace.destination_memory[i]== 0) continue;
    trace.destination_memory[i] += shift * trace_id;
    trace.destination_memory[i] = trace.destination_memory[i] % range;
  }
  for (u32 i = 0; i < NUM_INSTR_SOURCES; i++) {
    if (trace.source_memory[i] == 0) continue;
    trace.source_memory[i] += shift * trace_id;
    trace.source_memory[i] = trace.source_memory[i] % range;
  }
  return ret;
}

void MultiTraceLoader::set_read_bound(s64 b) {
  _bound = b;
}
