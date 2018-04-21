#include "memory_hierarchy.h"
#include "cr_policy.h"

PolicyFactory::~PolicyFactory() {
  for (auto p: _policies) {
    assert(p != NULL);
    delete p;
  }
}

CRPolicyInterface* PolicyFactory::get_policy(CR_POLICY policy_type) {
  auto iter = _shared_policies.find(policy_type);
  if (iter != _shared_policies.end())
    return iter->second;

  auto ret = create_policy(policy_type);
  _shared_policies[policy_type] = ret;
  return ret;
}

CRPolicyInterface* PolicyFactory::create_policy(CR_POLICY policy_type) {
  CRPolicyInterface* ret = NULL;
  CacheBlockFactoryInterace *factory = NULL;
  switch (policy_type) {
    case LRU_POLICY:
      factory = new BaseBlockFactory();
      ret = new CR_LRU_Policy(factory);
      break;

    case RANDOM_POLICY:
      factory = new BaseBlockFactory();
      ret = new CRRandomPolicy(factory);
      break;

    case LIP_POLICY:
      factory = new BaseBlockFactory();
      ret = new CR_LIP_Policy(factory);
      break;

    default:
      assert(0);
      break;
  }
  _policies.push_back(ret);
  return ret;
}

CacheBlockBase* BaseBlockFactory::create(u64 tag, u64 blk_size, const MemoryAccessInfo &info) {
  CacheBlockBase *blk = new CacheBlockBase(info.addr, blk_size, tag);
  return blk;
}

CRRandomPolicy::CRRandomPolicy(CacheBlockFactoryInterace* factory): CRPolicyInterface(factory) {
  srand (time(NULL));
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

void CR_LIP_Policy::on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info) {
  (void)info;
  auto cand = line->get_block_by_pos(pos);
  for (u32 i = 0; i <= pos; i++) {
    auto to_evict = line->get_block_by_pos(i);
    line->evict_by_pos(i, cand, false);
    cand = to_evict;
  }
}

void CR_LIP_Policy::on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info){
  u32 ways = line->get_ways();
  auto cand = _factory->create(tag, line->get_block_size(), info);
  line->evict_by_pos(ways-1, cand, true);
}
