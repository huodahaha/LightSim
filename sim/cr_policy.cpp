#include "memory_hierarchy.h"
#include "cr_policy.h"
#include <algorithm>

#define BIP_BIMODAL_THROTTLE  1.0/16
#define PSEL_WIDTH 10
#define PSEL_MAX ((1<<PSEL_WIDTH)-1)
#define PSEL_THRS PSEL_MAX/2

PolicyFactory::~PolicyFactory() {
  for (auto p: _policies) {
    assert(p != NULL);
    delete p;
  }
}

CRPolicyInterface* PolicyFactory::get_policy(const MemoryConfig &config) {
  CR_POLICY policy_type = config.policy_type;
  auto iter = _shared_policies.find(policy_type);
  if (iter != _shared_policies.end())
    return iter->second;

  auto ret = create_policy(config);
  if (ret->is_shared()) {
    _shared_policies[policy_type] = ret;
  }
  return ret;
}

CRPolicyInterface* PolicyFactory::create_policy(const MemoryConfig &config) {
  CR_POLICY policy_type = config.policy_type;
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

    case BIP_POLICY:
      factory = new BaseBlockFactory();
      ret = new CR_BIP_Policy(factory);
      break;

    case DIP_POLICY:
      factory = new BaseBlockFactory();
      ret = new CR_DIP_Policy(factory, config.sets);
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

CR_BIP_Policy::CR_BIP_Policy(CacheBlockFactoryInterace* factory) : CRPolicyInterface(factory) {
  _throttle = BIP_BIMODAL_THROTTLE;
  srand(time(NULL));

  _lru = new CR_LRU_Policy(factory);
  _lip = new CR_LIP_Policy(factory);
}

CR_BIP_Policy::~CR_BIP_Policy() {
  delete _lru;
  delete _lip;
}

bool CR_BIP_Policy::use_LRU() {
  return ((rand()) / (double)RAND_MAX) < _throttle;
}

void CR_BIP_Policy::on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info) {
  if (use_LRU()) {
    _lru->on_arrive(line, tag, info);
  }
  else {
    _lip->on_arrive(line, tag, info);
  }
}

void CR_BIP_Policy::on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info) {
  _lru->on_hit(line, pos, info);
}

CR_DIP_Policy::CR_DIP_Policy(CacheBlockFactoryInterace* factory, u32 sets) : CRPolicyInterface(factory) {
  // start with LRU
  _PSEL = 0;
  _lru = new CR_LRU_Policy(factory);
  _bip = new CR_BIP_Policy(factory);

  if (sets < 4) {
    SIMLOG(SIM_ERROR, "cache need to have at least 4 sets for set dueling\n");
    exit(1);
  }

  vector<u32> all_sets;
  for (u32 i = 0; i < sets; i++) {
    all_sets.push_back(i);
    _sets_type.push_back(DIP_FOLLOWER);
  }
  random_shuffle(all_sets.begin(), all_sets.end());

  for (u32 i = 0; i < sets/4; i++) {
    u32 rand_idx = all_sets[i];
    _sets_type[rand_idx] = BIP_LEADER;
  }
  for (u32 i = sets/4; i < sets/2; i++) {
    u32 rand_idx = all_sets[i];
    _sets_type[rand_idx] = LRU_LEADER;
  }
}

CR_DIP_Policy::~CR_DIP_Policy() {
  delete _lru;
  delete _bip;
}

void CR_DIP_Policy::on_miss(CacheSet *line, const MemoryAccessInfo &info) {
  (void)info;
  u32 set_no = line->get_set_num();
  if ((_sets_type[set_no] == BIP_LEADER) && (_PSEL > 0)) {
    _PSEL--;
  }
  else if ((_sets_type[set_no] == LRU_LEADER) && (_PSEL < PSEL_MAX)) {
    _PSEL++;
  }
};

void CR_DIP_Policy::on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info) {
  u32 set_no = line->get_set_num();
  if (_sets_type[set_no] == BIP_LEADER) {
    _bip->on_arrive(line, tag, info);
  }
  else if (_sets_type[set_no] == LRU_LEADER) {
    _lru->on_arrive(line, tag, info);
  }
  else {
    // follower
    if (_PSEL > PSEL_THRS) {
      _bip->on_arrive(line, tag, info);
    }
    else {
      _lru->on_arrive(line, tag, info);
    }
  }
}

void CR_DIP_Policy::on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info) {
  _lru->on_hit(line, pos, info);
}
