#ifndef CACHE_REPLACEMENT_TRACE_LOADER_H
#define CACHE_REPLACEMENT_TRACE_LOADER_H

#include "inc_all.h"

#define NUM_INSTR_DESTINATIONS 2
#define NUM_INSTR_SOURCES 4

using namespace std;

struct TraceFormat {
  u64 pc;  // instruction pointer (program counter) value
  /******* to be added ******
   *  u8 opcode;
   *  u32 thread_id;
   *************************/
  u8 is_branch;    // is this branch
  u8 branch_taken; // if so, is this taken
  u8 destination_registers[NUM_INSTR_DESTINATIONS]; // output registers
  u8 source_registers[NUM_INSTR_SOURCES];           // input registers
  u64 destination_memory[NUM_INSTR_DESTINATIONS]; // output memory
  u64 source_memory[NUM_INSTR_SOURCES];           // input memory
  TraceFormat(); // just all zero initialization
};

class TraceLoader {
private:
  FILE * _trace_file;
  bool _reach_end;
public:
TraceLoader(string filename);
  ~TraceLoader();

  pair<bool, TraceFormat> next_instruction();
  bool is_end() const;
};



#endif //CACHE_REPLACEMENT_TRACE_LOADER_H
