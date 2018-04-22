#include "memory_hierarchy.h"

MemoryConfig::MemoryConfig(const CacheNodeCfg cfg, u32 priority_) {
  priority = priority_;
  latency = cfg.latency;

  ways = cfg.assoc;
  blk_size = cfg.blocksize;
  sets = cfg.sets;

  if (cfg.cr_policy == "LRU" || 
      cfg.cr_policy == "lru" ||
      cfg.cr_policy == "Lru") {
    policy_type = LRU_POLICY;
  }
  else if (cfg.cr_policy == "Random" || 
           cfg.cr_policy == "RANDOM" ||
           cfg.cr_policy == "random") {
    policy_type = RANDOM_POLICY;
  }
  else if (cfg.cr_policy == "lip" || 
           cfg.cr_policy == "LIP" ||
           cfg.cr_policy == "Lip") {
    policy_type = LIP_POLICY;
  }
  else {
    SIMLOG(SIM_ERROR, "unsupported policy type %s\n", cfg.cr_policy.c_str());
    exit(1);
  }
}

MemoryConfig::MemoryConfig(const MemoryNodeCfg cfg, u32 priority_) {
  priority = priority_;
  latency = cfg.latency;
}

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

#ifdef DEBUG
  SIMLOG(SIM_INFO, "handler: %s, type: %s\ttick: %lld\taddr: %llu\n", 
         get_tag().c_str(), event_type_to_string(type).c_str(), tick, memory_data->addr);
#endif

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

#ifdef DEBUG
  SIMLOG(SIM_INFO, "handler: %s, type: %s\ttick: %lld\taddr: %llu\n", 
         get_tag().c_str(), event_type_to_string(type).c_str(), tick, memory_data->addr);
#endif

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

CacheUnit::CacheUnit(const string &tag, const MemoryConfig &config)
  : MemoryUnit(tag, config.latency, config.priority), 
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
  auto stats_manager = MemoryStatsManagerObj::get_instance();
  auto stats = stats_manager->get_stats_handler(get_tag());
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

MainMemory::MainMemory(const string &tag, const MemoryConfig &config) :
    MemoryUnit(tag, config.latency, config.priority) {}

bool MainMemory::try_access_memory(const MemoryAccessInfo &info) {
  (void)info;
  return true;
}

void MainMemory::on_memory_arrive(const MemoryAccessInfo &info) {
  (void)info;
}

void MemoryStats::display(FILE *stream, const string &tag) {
  fprintf(stream, "cache tag: %s\n", tag.c_str());
  fprintf(stream, "cache hits %llu\n", _hits);
  fprintf(stream, "cache misses %llu\n\n", _misses);
}

void MemoryStats::clear() {
  _hits = 0;
  _misses = 0;
}

MemoryStatsManager::~MemoryStatsManager() {
  for (auto &entry: _stats_handlers) {
    delete entry.second;
  }
}

MemoryStats* MemoryStatsManager::get_stats_handler(const string &tag) {
  auto iter = _stats_handlers.find(tag);
  if (iter == _stats_handlers.end()) {
    auto handler = new MemoryStats();
    _stats_handlers[tag] = handler;
  }

  return _stats_handlers[tag];
}

void MemoryStatsManager::display_all(FILE *stream) {
  for (auto &entry: _stats_handlers) {
    entry.second->display(stream, entry.first);
  }
}

void CpuConnector::set_tracer(const vector<u64> &traces) {
  _traces = traces;
  _idx = 0;
}

bool CpuConnector::try_access_memory(const MemoryAccessInfo &info) {
  (void)info;
  return false;
}

void CpuConnector::on_memory_arrive(const MemoryAccessInfo &info) {
  if (! _waiting_event_data) return;
  auto iter = _pending_refs.find(info.addr);
  if (iter != _pending_refs.end()) {
    _pending_refs.erase(iter);
  }
#ifdef DEBUG
  else {
    assert(0);
  }
#endif
  if (_pending_refs.empty()) {
    auto evnet_queue = EventEngineObj::get_instance();
    Event *e = new Event(InstExecution, _cpu_ptr, _waiting_event_data);
    evnet_queue->register_after_now(e, 1, get_priority());
    _waiting_event_data = nullptr;
  }
}

void CpuConnector::issue_memory_access() {
  u64 addr = _traces[_idx++];
  MemoryAccessInfo info(addr, 0);
  issue_memory_access(info, nullptr);
}

void CpuConnector::issue_memory_access(const MemoryAccessInfo &info,
                                       CPUEventData *event_data) {
  auto evnet_queue = EventEngineObj::get_instance();
  MemoryEventData *d = new MemoryEventData(info);
  Event *e = new Event(MemoryOnAccess, this, d);
  evnet_queue->register_after_now(e, 0, get_priority());
  if (event_data) {
    _waiting_event_data = event_data;
    _pending_refs.insert(info.addr);
  }
}

MemoryUnit* PipeLineBuilder::create_node(BaseNodeCfg *cfg, u8 level) {
  auto iter = _nodes.find(cfg->name);
  if (iter == _nodes.end()) {
    MemoryUnit* cur_unit = NULL;
    MemoryUnit* next_unit = NULL;
    SequentialCPU * cpu_unit = NULL;
    switch (cfg->type) {
      case CpuNode: {
        //todo how to get the id??
        const u8 id = 0;
        cpu_unit = new SequentialCPU(cfg->name, id);
        break;
      }

      case CacheNode: {
        CacheNodeCfg* cache_cfg = (CacheNodeCfg *)cfg;
        MemoryConfig memcfg(*cache_cfg, level);
        cur_unit = new CacheUnit(cfg->name, memcfg);
        break;
      }

      case MemoryNode: {
        MemoryNodeCfg* cache_cfg = (MemoryNodeCfg*)cfg;
        MemoryConfig memcfg(*cache_cfg, level);
        cur_unit = new MainMemory(cfg->name, memcfg);
        break;
      }

      default: {
        SIMLOG(SIM_ERROR, "unsupported cfg type\n");
        exit(1);
      }
    }

    if (cfg->next_node != NULL) {
      next_unit = create_node(cfg->next_node, level + 1);
    }
    
    // assemble
    if (next_unit) {
      next_unit->add_prev(cur_unit);
      cur_unit->set_next(next_unit);
    }

    _nodes[cfg->name] = cur_unit;
  }
    
  return _nodes[cfg->name];
}

void PipeLineBuilder::load(const map<string, BaseNodeCfg*> &nodes_cfg) {
  if (_nodes_cfg.size() > 0) {
    SIMLOG(SIM_ERROR, "try to reload pipeline builder\n");
    exit(1);
  }
  
  _nodes_cfg = nodes_cfg;
}

PipeLineBuilder::~PipeLineBuilder() {
  for (auto &entry : _nodes) {
    delete entry.second;
  }
}

vector<CpuConnector* > PipeLineBuilder::get_connectors() {
  vector<CpuConnector* > cpus;
  vector<BaseNodeCfg*> cpu_cfgs;
  
  for (auto &entry: _nodes_cfg) {
    if (entry.second->type == CpuNode) {
      cpu_cfgs.push_back(entry.second);
    }
  }

  for (auto &cpu_cfg: cpu_cfgs) {
    CpuConnector *conn = (CpuConnector *)create_node(cpu_cfg, 0);
    cpus.push_back(conn);
  }

  return cpus;
}
