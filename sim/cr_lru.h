#include "memory_hierarchy.h"

class CR_LRU_BlockFactory: public CacheBlockFactoryInterace {
 public:
  CacheBlockBase* create(u64 addr, u64 tag, CacheSet *parent_set, u64 PC);
};

class CR_LRU_Policy: public CRPolicyInterface {
 public:
  CR_LRU_Policy(CacheBlockFactoryInterace* factory): CRPolicyInterface(factory) {};
  void on_hit(CacheSet *line, u32 pos, u64 addr, u64 PC);
  void on_arrive(CacheSet *line, u64 addr, u64 tag, u64 PC);
};
