#include "memory_hierarchy.h"

class CRRandomBlockFactory: public CacheBlockFactoryInterace {
 public:
  CacheBlockBase* create(u64 addr, u64 tag, CacheSet *parent_set, u64 PC);
};

class CRRandomPolicy: public CRPolicyInterface {
 public:
  CRRandomPolicy(CacheBlockFactoryInterace* factory): CRPolicyInterface(factory) {};
  void on_hit(CacheSet *line, u32 pos, u64 addr, u64 PC);
  void on_arrive(CacheSet *line, u64 addr, u64 tag, u64 PC);
};
