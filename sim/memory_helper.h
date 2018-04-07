#ifndef MEMORY_HELPER_H
#define MEMORY_HELPER_H

#include "inc_all.h"

extern const u64 MACHINE_WORD_SIZE;
extern const u64 MAX_SETS_SIZE;
extern const u64 MAX_BLOCK_SIZE;

inline bool check_addr_valid(u64 addr) {
  assert(MACHINE_WORD_SIZE > 0);
  // addr >> 64 is not permitted, so do the shift twice
  addr = addr >> 1;
  return (addr >> (MACHINE_WORD_SIZE-1)) == 0;
}

inline bool is_power_of_two(u64 num) {
  return (num & (num - 1)) == 0;
}

inline u32 len_of_binary(u64 num) {
  int ret = 0;
  while (num) {
    num = num >> 1;
    ret++;
  }
  return ret;
}

#endif
