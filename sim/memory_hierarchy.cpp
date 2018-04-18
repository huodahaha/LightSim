#include "memory_hierarchy.h"

MemoryEventData::MemoryEventData(const MemoryAccessInfo &info): 
    addr(info.addr), PC(info.PC) {};

MemoryAccessInfo::MemoryAccessInfo(const MemoryEventData &data):
    addr(data.addr), PC(data.PC) {};

CacheSet::CacheSet(u32 ways, u32 blk_size, u32 sets, CRPolicyInterface *policy) :_ways(ways), 
    _blk_size(blk_size), _sets(sets), _blocks(ways, NULL), _cr_policy(policy) {
  assert(_cr_policy);
  assert(_blk_size < MAX_BLOCK_SIZE);
}

CacheSet::CacheSet(u32 ways, u32 blk_size, u32 sets, CRPolicyInterface *policy, const string &tag): _ways(ways), 
    _blk_size(blk_size), _sets(sets), _blocks(ways, NULL), _cr_policy(policy), _set_tag(tag) {
  assert(_cr_policy);
  assert(_blk_size < MAX_BLOCK_SIZE);
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
  u32 s = len_of_binary(_sets);
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

void CacheSet::evict_by_pos(u32 pos, CacheBlockBase *blk, bool is_delete) {
  assert(pos < _ways);
  if (is_delete && _blocks[pos]) {
    delete _blocks[pos];
  }
  _blocks[pos] = blk;
}

CacheBlockBase* CacheSet::get_block_by_pos(u32 pos) {
  assert(pos < _ways);
  return _blocks[pos];
}

bool CacheSet::try_access_memory(const MemoryAccessInfo &info) {
  u64 tag = calulate_tag(info.addr);
  s32 pos = find_pos_by_tag(tag);
  if (pos == -1) {
    return false;
  }
  else {
    //printf("on hit\n");
    //print_blocks(stdout);
    _cr_policy->on_hit(this, pos, info);
    //print_blocks(stdout);
    return true;
  }
}

void CacheSet::on_memory_arrive(const MemoryAccessInfo &info) {
  u64 tag = calulate_tag(info.addr);
  //printf("on arrive\n");
  //print_blocks(stdout);
  _cr_policy->on_arrive(this, tag, info);
  //print_blocks(stdout);
}

void CacheSet::print_blocks(FILE* fs) {
  fprintf(fs, "set NO.%s:\t", _set_tag.c_str());
  for (auto blk: _blocks) {
    if (blk == NULL) {
      fprintf(fs, "null\t");
    }
    else {
      fprintf(fs, "%llu\t", blk->get_addr());
    }
  }
  fprintf(fs, "\n");
}

void MemoryUnit::proc(u64 tick, EventDataBase* data, EventType type) {
  (void)tick;
  MemoryEventData *memory_data = (MemoryEventData *)data;

  EventEngine *evnet_queue = EventEngineObj::get_instance();

  if (type == MemoryOnAccess) {
    auto iter = _pending_refs.find(memory_data->addr);
    if (iter != _pending_refs.end()) {
      return;
    }
    else {
      _pending_refs.insert(memory_data->addr);
    }

    MemoryAccessInfo access_info(*memory_data);
    bool ret = try_access_memory(access_info);
    if (ret == true) {
      for (auto prev_unit: _prev_units) {
        MemoryEventData *d = new MemoryEventData(*memory_data);
        Event *e = new Event(MemoryOnArrive, prev_unit, d);
        evnet_queue->register_after_now(e, get_latency(), prev_unit->get_priority());
      }
    }
    else {
      MemoryEventData *d = new MemoryEventData(*memory_data);
      Event *e = new Event(MemoryOnAccess, _next_unit, d);      
      evnet_queue->register_after_now(e, 1, _next_unit->get_priority());
    }
  }

  else if (type == MemoryOnArrive) {
    auto iter = _pending_refs.find(memory_data->addr);
    if (iter == _pending_refs.end()) {
      return;
    }
    else {
      _pending_refs.erase(memory_data->addr);
    }

    MemoryAccessInfo arrive_info(*memory_data);
    on_memory_arrive(arrive_info);
    for (auto prev_unit: _prev_units) {
      MemoryEventData *d = new MemoryEventData(*memory_data);
      Event *e = new Event(MemoryOnArrive, prev_unit, d);
      evnet_queue->register_after_now(e, get_latency(), prev_unit->get_priority());
    }
  }
}

bool MemoryUnit::validate(EventType type) {
  return ((type == MemoryOnAccess) || (type == MemoryOnArrive));
}

CacheUnit::CacheUnit(const MemoryConfig &config)
  : MemoryUnit(config.latency, config.proiority), 
    _ways(config.ways), _blk_size(config.blk_size), _sets(config.sets){
  auto factory = PolicyFactoryObj::get_instance();
  _cr_policy = factory->create_policy(config.policy_type);
  if (!_cr_policy) {
    SIMLOG(SIM_ERROR, "cache replacemenet policy can not be NULL");
    exit(1);
  }
  else if (_sets >= MAX_SETS_SIZE) {
    SIMLOG(SIM_ERROR, "sets number exceed system restriction");
    exit(1);
  }
  else if (_blk_size >= MAX_BLOCK_SIZE) {
    SIMLOG(SIM_ERROR, "block size exceed system restriction");
    exit(1);
  }
  else if (!is_power_of_two(_sets)) {
    SIMLOG(SIM_ERROR, "sets number should be power of 2");
    exit(1);
  }
  else if (!is_power_of_two(_blk_size)) {
    SIMLOG(SIM_ERROR, "block size should be power of 2");
    exit(1);
  }

  u32 s = len_of_binary(_sets);
  u32 b = len_of_binary(_sets);

  if (s + b >= MACHINE_WORD_SIZE) {
    SIMLOG(SIM_ERROR, "cache size too large");
    exit(1);
  }
  
  for (u32 i = 0; i < _sets; i++) {
    CacheSet *line = new CacheSet(_ways, _blk_size, _sets, _cr_policy);
    _cache_sets.push_back(line);
  }
}

CacheUnit::~CacheUnit() {
  for (u32 i = 0; i < _sets; i++) {
    delete _cache_sets[i];
  }
}

u64 CacheUnit::get_set_no(u64 addr) {
  u32 s = len_of_binary(_sets);
  u32 b = len_of_binary(_blk_size);
  if (s == 0)
    return 0;
  else
    return addr << (MACHINE_WORD_SIZE - s - b) >> (MACHINE_WORD_SIZE - s);
}

bool CacheUnit::try_access_memory(const MemoryAccessInfo &info) {
  u64 set_no = get_set_no(info.addr);
  assert(set_no < _cache_sets.size());
  auto cache_set = _cache_sets[set_no];
  auto ret = cache_set->try_access_memory(info);
  auto stats = MemoryStatsObj::get_instance();
  if (ret == true) {
    stats->increment_hit();
  }
  else {
    stats->increment_miss();
  }
  return ret;
}

void CacheUnit::on_memory_arrive(const MemoryAccessInfo &info) {
  u64 set_no = get_set_no(info.addr);
  assert(set_no < _cache_sets.size());
  auto cache_set = _cache_sets[set_no];
  cache_set->on_memory_arrive(info);
}

MainMemory::MainMemory(const MemoryConfig &config) :
    MemoryUnit(config.latency, config.proiority) {}

bool MainMemory::try_access_memory(const MemoryAccessInfo &info) {
  (void)info;
  return true;
}

void MainMemory::on_memory_arrive(const MemoryAccessInfo &info) {
  (void)info;
}

void MemoryStats::display(FILE *stream) {
  fprintf(stream, "cache hits %llu\n", _hits);
  fprintf(stream, "cache misses %llu\n", _misses);
}

void MemoryStats::clear() {
  _hits = 0;
  _misses = 0;
}

MemoryPipeLine::MemoryPipeLine(vector<MemoryConfig> &configs, MemoryUnit *alu) {
  assert(configs.size() > 0);

  // Assume we have the pipeline L1 -> L2 -> .... -> Main Memory 
  // the leftmost have the lowest priority, the right most have 
  // highest proiority
}

MemoryPipeLine::~MemoryPipeLine() {
  for (u32 i = 0; i < _units.size(); i++) {
    delete _units[i];
  }
}

bool MemoryPipeLine::try_access_memory(const MemoryAccessInfo &info) {
  return _head->try_access_memory(info);
}
