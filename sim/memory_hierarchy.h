#ifndef MEMORY_HIERARCHY
#define MEMORY_HIERARCHY

#include "inc_all.h"
#include "event_engine.h"
#include "memory_helper.h"

using namespace std;

class CacheBlockBase;
class CacheBlockFactoryInterace;
class CacheSet;
class CRPolicyInterface;
class MemoryInterface;
class CacheUnit;
class MainMemory;
class MemoryStats;

/**
 * Cache Block, reserved_info for complex cache replacement policy
 * cache block can be overriden for store more informations
 */
class CacheBlockBase {
 protected:
  u64             _addr; 
  u64             _blk_size;       // length of the block
  u64             _tag;
  CacheSet*      _parent_set;

  CacheBlockBase() {};

 public:
  CacheBlockBase(u64 addr, u32 blk_size, u64 tag, CacheSet *parent_set):
      _addr(addr), _blk_size(blk_size), _tag(tag), _parent_set(parent_set) {
    assert(is_power_of_two(blk_size));
    assert(parent_set);
  } 

  CacheBlockBase(const CacheBlockBase &other): 
      _addr(other._addr), _blk_size(other._blk_size), _tag(other._blk_size)
      , _parent_set(other._parent_set) {};

  virtual ~CacheBlockBase() {};

  inline u64 get_addr() {
    return _addr;
  }
  
  inline u64 get_blk_size() {
    return _blk_size;
  }

  inline u64 get_tag() {
    return _tag;
  }

  inline CacheSet* get_parent_set() {
    return _parent_set;
  }
};

/*
 * responsible for creating new cache blocks
 * should be pair with cache replacement policy
 * factory class should be a singleton for each cache replacement policy
 */
class CacheBlockFactoryInterace{
 public:
  CacheBlockFactoryInterace() {};
  virtual ~CacheBlockFactoryInterace() {};
  virtual CacheBlockBase* create(u64 addr, u64 tag, CacheSet *parent_set, u64 PC) = 0;
};

/*
 * Cache replacement policy
 * policy class should be a singleton for each cache replacement policy
 */
class CRPolicyInterface {
 protected:
  CacheBlockFactoryInterace *_factory;

 public:
  CRPolicyInterface(CacheBlockFactoryInterace *factory): _factory(factory) {};
  virtual ~CRPolicyInterface() {};
  virtual void on_hit(CacheSet *line, u32 pos, u64 addr, u64 PC) = 0;
  virtual void on_arrive(CacheSet *line, u64 addr, u64 tag, u64 PC) = 0;
};

/**
 * Cache Set, all cache blokcs are stored in vector rather than queue
 * to give more exexpansibility
 */
class CacheSet {
 private:
  u32                               _ways;
  u32                               _blk_size;
  u64                               _set_no;
  vector<CacheBlockBase *>          _blocks;
  CRPolicyInterface *               _cr_policy;
  CacheUnit *                       _parent_cache_unit;

  CacheSet() {};                        // forbid default constructor
  CacheSet(const CacheSet&) {};         // forbid copy constructor

  s32 find_pos_by_tag(u64 tag);

 public:
  CacheSet(u32 ways, u64 set_no, CacheUnit *unit);
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

  inline u64 get_set_no() {
    return _set_no;
  }

  u64 calulate_tag(u64 addr);

  // can evict an empty
  void evict_by_pos(u32 pos, CacheBlockBase *blk, bool is_delete = true);
  CacheBlockBase* get_block_by_pos(u32 pos);

  bool try_access_memory(u64 addr, u64 PC);
  void on_memory_arrive(u64 addr, u64 PC);

  void print_blocks(FILE* fs);
};


class MemoryInterface {
 public:
  virtual bool try_access_memory(u64 addr, u64 PC) = 0;
  virtual void on_memory_arrive(u64 addr, u64 PC) = 0;
  virtual ~MemoryInterface(){};
};


class CacheUnit: public MemoryInterface {
 private:
  u32                             _ways;
  u32                             _blk_size;
  u64                             _sets;
  vector<CacheSet*>               _cache_sets;
  CRPolicyInterface *             _cr_policy;

  CacheUnit() {};
  CacheUnit(const CacheUnit &) {};

 public:
  CacheUnit(u32 ways, u32 blk_size, u64 sets, CRPolicyInterface * policy);
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
  bool try_access_memory(u64 addr, u64 PC);
  void on_memory_arrive(u64 addr, u64 PC);
};

/**
 * Main Memory
 * assume there is no MMU, no page fault
 */
class MainMemory: public MemoryInterface {
 public:
  bool try_access_memory(u64 addr, u64 PC);
  void on_memory_arrive(u64 addr, u64 PC);
};

typedef Singleton <MainMemory> MainMemoryObj;

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

  void display(FILE *stream);
  void clear();
};

typedef Singleton <MemoryStats> MemoryStatsObj;

// one per core
// todo
class MemoryHierarchy {
  private:
   MemoryInterface* _entry;

  public:

};

// should be a singleton
// todo
class MemoryManager {
 private:
  int _cores;
  vector<MemoryHierarchy *> _memory_hierarchys;

 public:
  MemoryHierarchy* get_memory_by_core(int core_id) {
    assert(core_id >= 0);
    assert(core_id < _cores);
    return _memory_hierarchys[core_id];
  }

};

#endif
