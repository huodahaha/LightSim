#include "memory_hierarchy.h"

CacheSet::~CacheSet() {
  for (u32 i = 0; i < _ways; i++) {
    if (_blocks[i] == NULL) {
      continue;
    }
    else {
      delete _blocks[i];
    }
  }
}

s32 CacheSet::find_pos_by_tag(u64 tag) {
  for (u32 i = 0; i < _ways; i++) {
    if (_blocks[i] == NULL) {
      continue;
    }
    else if (_blocks[i]->get_tag() == tag) {
      return i;
    }
  }
  return -1;
}

void CacheSet::evict_by_pos(u32 pos, CacheBlock *blk) {
  assert(pos < _ways);
  assert(blk->get_tag() == );
  if (_blocks[pos]) {
    delete _blocks[pos];
  }
  _blocks[pos] = blk;
}

bool CacheSet::try_access_memory(u64 addr, u64 PC, void* reserved) {
  s32 ret = find_pos_by_tag();
}
void on_memory_arrive(u64 addr, u64 PC, void* reserved);

u64 CacheSet::calulate_tag(u64 addr) {
  u32 s = len_of_binary(_sets);
  u32 b = len_of_binary(_blk_size);
  _tag = addr >> (s+b);
  _set_no = addr << (MACHINE_WORD_SIZE - s - b) >> (64 - s);
}


u64 CacheUnit::get_set_no() {
  u32 s = len_of_binary(_sets);
  u32 b = len_of_binary(_blk_size);
  _tag = addr >> (s+b);
  _set_no = addr << (MACHINE_WORD_SIZE - s - b) >> (64 - s);
}

bool CacheUnit::try_access_memory(u64 addr, u64 PC, void* reserved) {
  auto iter = 
}

void CacheUnit:on_memory_arrive(u64 addr, u64 PC, void* reserved) {

}



bool MainMemory::try_access_memory(u64 addr, u64 PC, void* reserved) {
  (void)addr;
  (void)PC;
  (void)reserved;
  return true;
}

void MainMemory:on_memory_arrive(u64 addr, u64 PC, void* reserved) {
  (void)addr;
  (void)PC;
  (void)reserved;
}

