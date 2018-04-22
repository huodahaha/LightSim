#ifndef MEMORY_HIERARCHY
#define MEMORY_HIERARCHY

#include <unordered_set>

#include "inc_all.h"
#include "event_engine.h"
#include "cfg_loader.h"
#include "ooo_cpu.h"

using namespace std;

struct MemoryConfig;
struct MemoryEventData;
struct MemoryAccessInfo;
struct CPUEventData;
class CacheBlockBase;
class CacheBlockFactoryInterace;
class CacheSet;
class CRPolicyInterface;
class MemoryInterface;
class MemoryUnit;
class CacheUnit;
class MainMemory;
class MemoryStats;
class SequentialCPU;

/*********************************  Enums  ********************************/
enum CR_POLICY {
  None,
  LRU_POLICY,
  RANDOM_POLICY,
  LIP_POLICY,
  BIP_POLICY,
  DIP_POLICY,
  POLICY_CNT
};
/**************************************************************************/

/*********************************  DTO   ********************************/

// cache unit
//  configuration
struct MemoryConfig {
  // both cache and main memory contains
  u8            priority;
  u32           latency;

  // only cache contains
  u32           ways;
  u32           blk_size;
  u64           sets;
  CR_POLICY     policy_type;

  MemoryConfig() {};
  MemoryConfig(u8 priority_, u32 latency_) : priority(priority_), latency(latency_) {};
  MemoryConfig(u8 priority_, u32 latency_, u32 ways_, u32 blk_size_, u64 sets_, 
               CR_POLICY policy_type_) : priority(priority_), latency(latency_), 
               ways(ways_), blk_size(blk_size_), sets(sets_), policy_type(policy_type_) {};
  MemoryConfig(const CacheNodeCfg cfg, u32 priority_);
  MemoryConfig(const MemoryNodeCfg cfg, u32 priority_);
};

struct MemoryEventData : public EventDataBase {
  u64 addr;
  u64 PC;

  MemoryEventData(u64 addr_, u64 PC_) : addr(addr_), PC(PC_) {};
  MemoryEventData(const MemoryAccessInfo &info);
};

struct MemoryAccessInfo {
  u64 addr;
  u64 PC;

  MemoryAccessInfo(u64 addr_, u64 PC_) : addr(addr_), PC(PC_) {};
  MemoryAccessInfo(const MemoryEventData &data);
};

/**************************************************************************/


/********************************  Objects ********************************/
class CacheBlockBase {
 protected:
  u64             _addr; 
  u32             _blk_size;       // length of the block
  u64             _tag;

  CacheBlockBase() {};

 public:
  CacheBlockBase(u64 addr, u32 blk_size, u64 tag):
      _addr(addr), _blk_size(blk_size), _tag(tag) {
        assert(is_power_of_two(blk_size));
      } 

  CacheBlockBase(const CacheBlockBase &other): 
      _addr(other._addr), _blk_size(other._blk_size), _tag(other._blk_size) {};

  virtual ~CacheBlockBase() {};

  inline u64 get_addr() {
    return _addr;
  }

  inline u32 get_blk_size() {
    return _blk_size;
  }

  inline u64 get_tag() {
    return _tag;
  }
};

class CacheBlockFactoryInterace{
 public:
  CacheBlockFactoryInterace() {};
  virtual ~CacheBlockFactoryInterace() {};
  virtual CacheBlockBase* create(u64 tag, u64 blk_size, const MemoryAccessInfo &info) = 0;
};

class CRPolicyInterface {
 protected:
  CacheBlockFactoryInterace *_factory;

 public:
  CRPolicyInterface(CacheBlockFactoryInterace *factory): _factory(factory) {};
  virtual ~CRPolicyInterface() {};
  virtual void on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info) = 0;
  virtual void on_miss(CacheSet *line, const MemoryAccessInfo &info);
  virtual void on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info) = 0;
  // some cache replacement policy need to store private information, make the
  // policy unsharable
  virtual bool is_shared();
};

class PolicyFactory {
 private:
  map<CR_POLICY, CRPolicyInterface*>      _shared_policies;
  vector<CRPolicyInterface*>              _policies;
  CRPolicyInterface* create_policy(const MemoryConfig &config);

 public:
  PolicyFactory() {};
  ~PolicyFactory();

  CRPolicyInterface* get_policy(const MemoryConfig &config);
};


/**
 * Cache Set, all cache blokcs are stored in vector rather than queue
 * to give more exexpansibility
 */
class CacheSet {
 private:
  u32                               _ways;
  u32                               _blk_size;
  u32                               _sets;
  u32                               _set_num = 0;   // not necessary, only used for set dueling
  vector<CacheBlockBase *>          _blocks;
  CRPolicyInterface *               _cr_policy;
  // for verbose output
  string                            _set_tag;

  CacheSet() {};                        // forbid default constructor
  CacheSet(const CacheSet&) {};         // forbid copy constructor

  s32 find_pos_by_tag(u64 tag);

 public:
  CacheSet(u32 ways, u32 blk_size, u32 sets, CRPolicyInterface *policy);
  CacheSet(u32 ways, u32 blk_size, u32 sets, CRPolicyInterface *policy, const string &tag);
  ~CacheSet();

  inline const vector<CacheBlockBase *>& get_all_blocks() {
    return _blocks;
  }

  inline u32 get_ways() {
    return _ways;
  }

  inline u32 get_block_size() {
    return _blk_size;
  }

  inline u32 get_set_num() {
    return _set_num;
  }

  void set_set_num(u32 set_num);

  u64 calulate_tag(u64 addr);

  // can evict an empty
  void evict_by_pos(u32 pos, CacheBlockBase *blk, bool is_delete = true);
  CacheBlockBase* get_block_by_pos(u32 pos);

  bool try_access_memory(const MemoryAccessInfo &info);
  void on_memory_arrive(const MemoryAccessInfo &info);

  void print_blocks(FILE* fs);
};

class MemoryInterface : public EventHandler {
 protected:
  virtual bool try_access_memory(const MemoryAccessInfo &info) = 0;
  virtual void on_memory_arrive(const MemoryAccessInfo &info) = 0;
 public:
  MemoryInterface(const string &tag) : EventHandler(tag) {};
};

class MemoryUnit : public MemoryInterface {
 private:
  // for LLC there are multiple previous memory units
  vector<MemoryUnit*>     _prev_units;
  MemoryUnit *            _next_unit;
  
  // since there could be more than one previous memory units
  // we store all pending address to identify if need process a 
  // memory on arrive event
  unordered_set<u64>      _pending_refs;

 protected:
  // for event engine
  u32                     _latency;
  u8                      _priority;

  void proc(u64 tick, EventDataBase* data, EventType type);
  bool validate(EventType type);

 public:
  MemoryUnit(string tag, u32 latency, u8 priority) : MemoryInterface(tag),
    _next_unit(NULL), _latency(latency), _priority(priority) {};

  virtual ~MemoryUnit(){};

  inline u32 get_latency() {
    return _latency;
  }

  inline u8 get_priority() {
    return _priority;
  }

  inline void add_prev(MemoryUnit *p) {
    _prev_units.push_back(p);
  }

  inline void set_next(MemoryUnit *n) {
    _next_unit = n;
  }
};

class CacheUnit: public MemoryUnit {
 private:
  // for memory
  u32                             _ways;
  u32                             _blk_size;
  u64                             _sets;
  CRPolicyInterface *             _cr_policy;
  vector<CacheSet*>               _cache_sets;

 protected:
  bool try_access_memory(const MemoryAccessInfo &info);
  void on_memory_arrive(const MemoryAccessInfo &info);

 public:
  CacheUnit(const string &tag, const MemoryConfig &config);
  ~CacheUnit ();

  inline u64 get_sets() {
    return _sets;
  }

  inline u32 get_blk_size() {
    return _blk_size;
  }

  inline CRPolicyInterface* get_policy() {
    return _cr_policy;
  }

  u64 get_set_no(u64 addr);
};

/**
 * Main Memory
 * assume there is no MMU, no page fault
 */
class MainMemory: public MemoryUnit {
 protected:
  bool try_access_memory(const MemoryAccessInfo &info);
  void on_memory_arrive(const MemoryAccessInfo &info);

 public:
  MainMemory(const string &tag, const MemoryConfig &config);
};

class MemoryStats {
 private:
  u64 _misses;
  u64 _hits;

 public:
  MemoryStats(): _misses(0), _hits(0) {};

  inline void increment_miss() {
    _misses++;
  }

  inline void increment_hit() {
    _hits++;
  }

  void display(FILE *stream, const string &tag);
  void clear();
};

class MemoryStatsManager {
 private:
  map<string, MemoryStats*>     _stats_handlers;

 public:
  MemoryStatsManager() {};
  ~MemoryStatsManager();

  MemoryStats* get_stats_handler(const string &tag);
  void display_all(FILE *stream);
};

class CpuConnector: public MemoryUnit {
 private:
  vector<u64>   _traces;
  u32           _idx;
  unordered_set<u64> _pending_refs;
  CPUEventData *_waiting_event_data;
  SequentialCPU * _cpu_ptr;
 protected:
  bool try_access_memory(const MemoryAccessInfo &info);
  void on_memory_arrive(const MemoryAccessInfo &info);

 public:
  CpuConnector(const string &tag, u8 id);
  ~CpuConnector();
  void set_tracer(const vector<u64> &traces);
  void issue_memory_access();
  void issue_memory_access(const MemoryAccessInfo &info, CPUEventData *);
  void start();
//  void proc(u64 tick, EventDataBase* data, EventType type);
};


class PipeLineBuilder {
 private:
  map<string, BaseNodeCfg*>   _nodes_cfg;
  map<string, MemoryUnit*>   _nodes;

  MemoryUnit* create_node(BaseNodeCfg *cfg, u8 level);

 public:
  PipeLineBuilder() {};
  void load(const map<string, BaseNodeCfg*> &nodes_map);
  ~PipeLineBuilder();

  vector<CpuConnector* > get_connectors();
};

/**************************************************************************/

/********************************  Singleton ******************************/

typedef Singleton<PolicyFactory> PolicyFactoryObj;
typedef Singleton<MemoryStatsManager> MemoryStatsManagerObj;
typedef Singleton<PipeLineBuilder> PipeLineBuilderObj;

/**************************************************************************/

#endif
