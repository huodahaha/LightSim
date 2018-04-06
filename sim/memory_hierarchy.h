#ifndef MEMORY_HIERARCHY
#define MEMORY_HIERARCHY

#include "inc_all.h"
#include "event_queue.h"

using namespace std;

typedef long long MemoryAddr;

class IMemoryUnit {
 public:
  virtual int access_memory(MemoryAddr addr) = 0;
};


// cache repalcement policy
class ICRPolicy {
 private:
  Cache* _cache;

  ICRPolicy() {};


 public:
  ICRPolicy(Cache *c) {

  }

}

class Cache: public IMemoryUnit {
 private:
  

};

class MainMemory: public IMemoryUnit {

};

// one per core
class MemoryHierarchy {
  private:
   IMemoryUnit* _entry;

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
