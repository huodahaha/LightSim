#include "memory_hierarchy.h"

class CR_LRU_BlockFactory: public CacheBlockFactoryInterace {
 public:
  CacheBlockBase* create(u64 tag, u64 blk_size, const MemoryAccessInfo &info);
};

class CR_LRU_Policy: public CRPolicyInterface {
 public:
  CR_LRU_Policy(CacheBlockFactoryInterace* factory): CRPolicyInterface(factory) {};
  void on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info);
  void on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info);
};
