#include "memory_hierarchy.h"

class CRRandomBlockFactory: public CacheBlockFactoryInterace {
 public:
  CacheBlockBase* create(u64 tag, u64 blk_size, const MemoryAccessInfo &info);
};

class CRRandomPolicy: public CRPolicyInterface {
 public:
  CRRandomPolicy(CacheBlockFactoryInterace* factory): CRPolicyInterface(factory) {};
  void on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info);
  void on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info);
};
