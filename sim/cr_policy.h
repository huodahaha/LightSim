#ifndef CR_POLICY_H
#define CR_POLICY_H

#include "memory_hierarchy.h"

class BaseBlockFactory: public CacheBlockFactoryInterace {
 public:
  CacheBlockBase* create(u64 tag, u64 blk_size, const MemoryAccessInfo &info);
};

class CR_LRU_Policy: public CRPolicyInterface {
 public:
  CR_LRU_Policy(CacheBlockFactoryInterace* factory): CRPolicyInterface(factory) {};
  void on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info);
  void on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info);
};

class CRRandomPolicy: public CRPolicyInterface {
 public:
  CRRandomPolicy(CacheBlockFactoryInterace* factory);
  void on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info);
  void on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info);
};

class CR_LIP_Policy: public CRPolicyInterface {
 public:
  CR_LIP_Policy(CacheBlockFactoryInterace* factory): CRPolicyInterface(factory) {};
  void on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info);
  void on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info);
};

class CR_BIP_Policy: public CRPolicyInterface {
 private:
  double      _throttle;

  bool do_replace();

 public:
  CR_BIP_Policy(CacheBlockFactoryInterace* factory);
  void on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info);
  void on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info);
};

#endif
