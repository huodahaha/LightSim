#ifndef CACHE_REPLACEMENT_TRACE_LOADER_H
#define CACHE_REPLACEMENT_TRACE_LOADER_H

#include "inc_all.h"
#include "util.h"
#define NUM_INSTR_DESTINATIONS 2
#define NUM_INSTR_SOURCES 4
#define LONGEST_OP_CODE_STRING 16

using namespace std;

struct __attribute__ ((packed)) TraceFormat {
  // i have not idea why use or not using marco will affect the sizeof(TraceFormat)
  // this way works
  unsigned long long      pc;  // instruction pointer (program counter) value
  // unsigned int            opcode; // opcode of the instruction
  // char                    opcode_string[LONGEST_OP_CODE_STRING];
  // unsigned int            thread_id; // system thread id
  unsigned char           is_branch;    // is this branch
  unsigned char           branch_taken; // if so, is this taken
  unsigned char           destination_registers[NUM_INSTR_DESTINATIONS]; // output registers
  unsigned char           source_registers[NUM_INSTR_SOURCES];           // input registers
  unsigned long long int  destination_memory[NUM_INSTR_DESTINATIONS]; // output memory
  unsigned long long int  source_memory[NUM_INSTR_SOURCES];           // input memory

//  this way dose not work as sizeof(TraceFormat) increase by 8 mysteriously
//  u64 pc;  // instruction pointer (program counter) value
//  u32 opcode; // opcode of the instruction
//  u32 thread_id; // system thread id
//  u8 is_branch;    // is this branch
//  u8 branch_taken; // if so, is this taken
//  u8 destination_registers[NUM_INSTR_DESTINATIONS]; // output registers
//  u8 source_registers[NUM_INSTR_SOURCES];           // input registers
//  u64 destination_memory[NUM_INSTR_DESTINATIONS]; // output memory
//  u64 source_memory[NUM_INSTR_SOURCES];           // input memory

  TraceFormat();
};


class TraceLoader {
 private:
  FILE *      _trace_file;
  s64         _bound = -1;
  s64         _count = 0;
  TraceLoader() {}
 public:
  TraceLoader(const string &filename);
  ~TraceLoader();
  void set_read_bound(s64 bound);
  size_t next_instruction(TraceFormat &trace);
};


class MultiTraceLoader {
 private:
  vector<TraceLoader *>   _trace_loaders;
  size_t                  _cur_assigned;
  s64                     _bound = -1;
 public:
  MultiTraceLoader(): _trace_loaders(0), _cur_assigned(0) {};
  ~MultiTraceLoader();
  void adding_trace(const string &filename);
  size_t get_trace_num() const;
  s32 assign_trace();
  size_t next_instruction(u32 trace_id, TraceFormat &trace);
  void set_read_bound(s64);
};


//class TraceLoader
typedef Singleton<MultiTraceLoader> MultiTraceLoaderObj;
#endif //CACHE_REPLACEMENT_TRACE_LOADER_H
