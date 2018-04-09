#include "cr_lru.h"

CacheBlockBase* CR_LRU_BlockFactory::create(u64 tag, u64 blk_size, const MemoryAccessInfo &info) {
  CacheBlockBase *blk = new CacheBlockBase(info.addr, blk_size, tag);
  return blk;
}

void CR_LRU_Policy::on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info) {
  (void)info;
  auto cand = line->get_block_by_pos(pos);
  for (u32 i = 0; i <= pos; i++) {
    auto to_evict = line->get_block_by_pos(i);
    line->evict_by_pos(i, cand, false);
    cand = to_evict;
  }
}

void CR_LRU_Policy::on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info) {
  u32 ways = line->get_ways();
  auto cand = _factory->create(tag, line->get_block_size(), info);
  for (u32 i = 0; i < ways; i++) {
    auto to_evict = line->get_block_by_pos(i);
    line->evict_by_pos(i, cand, false);
    cand = to_evict;
  }
  
  delete cand;
}
