#include "cr_random.h"

CacheBlockBase* CRRandomBlockFactory::create(u64 tag, u64 blk_size, const MemoryAccessInfo &info) {
  CacheBlockBase *blk = new CacheBlockBase(info.addr, blk_size, tag);
  return blk;
}

void CRRandomPolicy::on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info) {
  // do nothing when a hit
  (void)line, (void)pos, (void)info;
}

void CRRandomPolicy::on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info) {
  auto blocks = line->get_all_blocks();
  u32 victim = rand()% blocks.size(); 
  auto new_block = _factory->create(tag, line->get_block_size(), info);
  
  for (u32 i = 0; i < blocks.size(); i++) {
    if (blocks[i] == NULL) {
      victim = i;
      break;
    }
  }

  line->evict_by_pos(victim, new_block, true);
}
