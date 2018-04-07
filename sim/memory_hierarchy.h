#ifndef MEMORY_HIERARCHY
#define MEMORY_HIERARCHY

#include "inc_all.h"
#include "event_queue.h"
#include "memory_helper.h"

using namespace std;

class CacheBlockBase;
class CacheBlockFactoryInterace;
class CacheSet;
class CRPolicyInterface;
class MemoryInterface;
class CacheUnit;
class MainMemory;

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
  CacheBlockBase(const CacheBlockBase &) {};

 public:
  CacheBlockBase(u64 addr, u32 blk_size, u64 tag, CacheSet *parent_set):
      _addr(addr), _blk_size(blk_size), _tag(tag), _parent_set(parent_set) {
    assert(is_power_of_two(blk_size));
    assert(is_power_of_two(sets));
    assert(parent_set);
  } 

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
};

/*
 * responsible for creating new cache blocks
 * should be pair with cache replacement policy
 */
class CacheBlockFactoryInterace {
 protected:
  u32     policy_id;

 public:
  virtual CacheBlockBase* create(u64 addr, u64 PC, void* reserved) = 0;
};

/*
 * Cache replacement policy
 */
class CRPolicyInterface {
 public:
  // this interface will need to refactor to provide more informations
  virtual void on_hit(CacheSet *line, u32 pos, u64 addr, u64 PC, void* reserved) = 0;
  virtual void on_miss(CacheSet *line, u32 tag, u64 addr, u64 PC, void* reserved) = 0;
};

/**
 * Cache Set, all cache blokcs are stored in vector rather than queue
 * to give more exexpansibility
 */
class CacheSet {
 private:
  u32                       _ways;
  u64                       _set_no;
  vector<CacheBlockBase *>      _blocks;
  CRPolicyInterface *               _cr_policy;
  CacheUnit *               _parent_cache_unit;

  CacheSet() {};
  CacheSet(const CacheSet&) {};

  s32 find_pos_by_tag(u64 tag);

 public:
  CacheSet(u32 ways, u64 sets) :_ways(ways), _sets(sets), _blocks(ways, NULL) {};
  ~CacheSet();

  inline const vector<CacheBlockBase *>& get_all_blocks() {
    return _blocks;
  }

  inline u64 get_set_no() {
    return _set_no;
  }

  u64 calulate_tag(u64 addr) {};

  // can evict an empty
  void evict_by_pos(u32 pos, CacheBlockBase *blk);

  bool try_access_memory(u64 addr, u64 PC, void* reserved);
  void on_memory_arrive(u64 addr, u64 PC, void* reserved);
};


class MemoryInterface {
 public:
  virtual bool try_access_memory(u64 addr, u64 PC, void* reserved) = 0;
  virtual void on_memory_arrive(u64 addr, u64 PC, void* reserved) = 0;
};


class CacheUnit: public MemoryInterface {
 private:
  u32                             _blk_size;
  u64                             _sets;
  vector<u64, CacheSet*>          _cache_lines;
  CRPolicyInterface *             _cr_policy;
  CacheBlockFactoryInterace  *    _block_factory;

 public:
  inline u64 get_sets() {
    return _sets;
  }

  inline u32 get_blk_size() {
    return _blk_size;
  }

  u64 get_set_no();
  bool try_access_memory(u64 addr, u64 PC, void* reserved);
  void on_memory_arrive(u64 addr, u64 PC, void* reserved);
};

/**
 * Main Memory
 * assume there is no MMU, no page fault
 */
class MainMemory: public MemoryInterface {
 public:
  bool try_access_memory(u64 addr, u64 PC, void* reserved);
  void on_memory_arrive(u64 addr, u64 PC, void* reserved);
};

// one per core
class MemoryHierarchy {
  private:
   MemoryInterface* _entry;

  public:


};

// one per system
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
