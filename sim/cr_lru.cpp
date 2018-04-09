#include "cr_lru.h"

CacheBlockBase* CR_LRU_BlockFactory::create(u64 addr, u64 tag, CacheSet *parent_set, u64 PC) {
  (void)PC;
  CacheBlockBase *blk = new CacheBlockBase(addr, parent_set->get_block_size(), tag, parent_set);
  return blk;
}

void CR_LRU_Policy::on_hit(CacheSet *cache_set, u32 pos, u64 addr, u64 PC) {
  (void)addr, (void)PC;
  auto cand = cache_set->get_block_by_pos(pos);
  for (u32 i = 0; i <= pos; i++) {
    auto to_evict = cache_set->get_block_by_pos(i);
    cache_set->evict_by_pos(i, cand, false);
    cand = to_evict;
  }
}

void CR_LRU_Policy::on_arrive(CacheSet *cache_set, u64 addr, u64 tag, u64 PC) {
  (void)addr, (void)PC;
  u32 ways = cache_set->get_ways();
  auto cand = _factory->create(addr, tag, cache_set, PC);
  for (u32 i = 0; i < ways; i++) {
    auto to_evict = cache_set->get_block_by_pos(i);
    cache_set->evict_by_pos(i, cand, false);
    cand = to_evict;
  }
  delete cand;
}
