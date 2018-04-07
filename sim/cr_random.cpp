#include "cr_random.h"

#define RANDOM_POLICY_ID 887799       // magic number for validation

CRRandomBlockFactory CRRandomBlockFactory::_instance(RANDOM_POLICY_ID);
CRRandomPolicy CRRandomPolicy::_instance(RANDOM_POLICY_ID);

CacheBlockFactoryInterace* CRRandomBlockFactory::get_instance() {
  return &_instance;
}

CacheBlockBase* CRRandomBlockFactory::create(u64 addr, u64 tag, CacheSet *parent_set, u64 PC) {
  (void)PC;
  CacheBlockBase *blk = new CacheBlockBase(addr, parent_set->get_block_size(), tag, parent_set);
  return blk;
}

CRPolicyInterface* CRRandomPolicy::get_instance(){
  return &_instance;
}

void CRRandomPolicy::on_hit(CacheSet *cache_set, u32 pos, u64 addr, u64 PC) {
  // do nothing when a hit
  (void)cache_set, (void)pos;
  (void)addr, (void)PC;
}

void CRRandomPolicy::on_miss(CacheSet *cache_set, u64 addr, u64 tag, u64 PC) {
  auto blocks = cache_set->get_all_blocks();
  u32 victim = rand()% blocks.size(); 
  auto new_block = _factory->create(addr, tag, cache_set, PC);
  
  for (u32 i = 0; i < blocks.size(); i++) {
    if (blocks[i] == NULL) {
      victim = i;
      break;;
    }
  }

  cache_set->evict_by_pos(victim, new_block);
}

