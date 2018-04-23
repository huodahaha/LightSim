#include "util.h"

const u64 MACHINE_WORD_SIZE = 64;
const u64 MAX_SETS_SIZE = 65536;
const u64 MAX_BLOCK_SIZE = 65536;

static bool VERBOSE = false;

void set_verbose() {
  VERBOSE = true;
}

bool is_verbose() {
  return VERBOSE;
}

extern bool VERBOSE;
