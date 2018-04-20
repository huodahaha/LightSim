//
// Created by Ma Huri on 4/13/18.
//

#include "trace_loader.h"

TraceFormat::TraceFormat() : pc(0), opcode(0), thread_id(0),
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

TraceLoader::TraceLoader(string filename) {
  char gzip_command[512];
  sprintf(gzip_command, "gunzip -c %s", filename.c_str());
  _trace_file = popen(gzip_command, "r");
  if (_trace_file == nullptr) {
    SIMLOG(SIM_ERROR, "\nUnable to read the trace file %s, exiting.\n", filename.c_str());
    exit(-1);
  }
}

TraceLoader::~TraceLoader() { pclose(_trace_file); }

size_t TraceLoader::next_instruction(TraceFormat &trace) {
  return fread(&trace, sizeof(TraceFormat), 1, _trace_file);
}

