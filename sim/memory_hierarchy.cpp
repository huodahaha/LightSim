#include "memory_hierarchy.h"

bool PolicyComponent::check_compatible(PolicyComponent* other) {
  return _policy_id == other->get_policy_id();
}

CacheSet::CacheSet(u32 ways, u64 set_no, CacheUnit *unit) :_ways(ways), _set_no(set_no), _blocks(ways, NULL), _parent_cache_unit(unit) {
  assert(_parent_cache_unit != NULL);
  _blk_size = unit->get_blk_size();
  _cr_policy = unit->get_policy();
}

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

u64 CacheSet::calulate_tag(u64 addr) {
  u32 s = len_of_binary(_parent_cache_unit->get_sets());
  u32 b = len_of_binary(_blk_size);
  return addr >> (s+b);
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

void CacheSet::evict_by_pos(u32 pos, CacheBlockBase *blk) {
  assert(pos < _ways);
  if (_blocks[pos]) {
    delete _blocks[pos];
  }
  _blocks[pos] = blk;
}

bool CacheSet::try_access_memory(u64 addr, u64 PC) {
  u64 tag = calulate_tag(addr);
  s32 pos = find_pos_by_tag(tag);
  if (pos == -1) {
    return false;
  }
  else {
    _cr_policy->on_hit(this, pos, addr, PC);
    return true;
  }
}

void CacheSet::on_memory_arrive(u64 addr, u64 PC) {
  u64 tag = calulate_tag(addr);
  _cr_policy->on_miss(this, addr, tag, PC);
}

u64 CacheUnit::get_set_no(u64 addr) {
  u32 s = len_of_binary(_sets);
  u32 b = len_of_binary(_blk_size);
  return addr << (MACHINE_WORD_SIZE - s - b) >> (64 - s);
}

bool CacheUnit::try_access_memory(u64 addr, u64 PC) {
  u64 set_no = get_set_no(addr);
  assert(set_no < _cache_sets.size());
  auto cache_set = _cache_sets[set_no];
  return cache_set->try_access_memory(addr, PC);
}

void CacheUnit::on_memory_arrive(u64 addr, u64 PC) {
  u64 set_no = get_set_no(addr);
  assert(set_no < _cache_sets.size());
  auto cache_set = _cache_sets[set_no];
  cache_set->on_memory_arrive(addr, PC);
}

bool MainMemory::try_access_memory(u64 addr, u64 PC) {
  (void)addr, (void)PC;
  return true;
}

void MainMemory::on_memory_arrive(u64 addr, u64 PC) {
  (void)addr, (void)PC;
}

