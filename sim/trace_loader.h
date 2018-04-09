#ifndef CACHE_REPLACEMENT_TRACE_LOADER_H
#define CACHE_REPLACEMENT_TRACE_LOADER_H

#include "inc_all.h"

#define NUM_INSTR_DESTINATIONS 2
#define NUM_INSTR_SOURCES 4
#define LONGEST_OP_CODE_STRING 16

using namespace std;

struct __attribute__ ((packed)) TraceFormat {
  // i have not idea why use or not using marco will affect the sizeof(TraceFormat)
  // this way works
  unsigned long long pc;  // instruction pointer (program counter) value
  unsigned int opcode; // opcode of the instruction
  char opcode_string[LONGEST_OP_CODE_STRING];
  unsigned int thread_id; // system thread id
  unsigned char is_branch;    // is this branch
  unsigned char branch_taken; // if so, is this taken
  unsigned char destination_registers[NUM_INSTR_DESTINATIONS]; // output registers
  unsigned char source_registers[NUM_INSTR_SOURCES];           // input registers
  unsigned long long int destination_memory[NUM_INSTR_DESTINATIONS]; // output memory
  unsigned long long int source_memory[NUM_INSTR_SOURCES];           // input memory

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
  FILE * _trace_file;
  bool _reach_end;
  TraceLoader() {}
public:
TraceLoader(string filename);
  ~TraceLoader();

  pair<bool, TraceFormat> next_instruction();
  bool is_end() const;
};



#endif //CACHE_REPLACEMENT_TRACE_LOADER_H
