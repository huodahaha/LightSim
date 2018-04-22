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
  double              _throttle;
  CRPolicyInterface*  _lru;
  CRPolicyInterface*  _lip;

  bool use_LRU();

 public:
  CR_BIP_Policy(CacheBlockFactoryInterace* factory);
  ~CR_BIP_Policy();
  void on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info);
  void on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info);
};

enum DIP_SET_TYPE {
  BIP_LEADER,
  LRU_LEADER,
  DIP_FOLLOWER,
};

class CR_DIP_Policy: public CRPolicyInterface {
 private:
  // 0(LRU) --- PESL_THRA --- (PESL_MAX)BIP
  s32                     _PSEL;
  CRPolicyInterface*      _lru;
  CRPolicyInterface*      _bip;
  vector<DIP_SET_TYPE>    _sets_type;

 public:
  CR_DIP_Policy(CacheBlockFactoryInterace* factory, u32 sets);
  ~CR_DIP_Policy();
  bool is_shared() {return false;};
  void on_miss(CacheSet *line, const MemoryAccessInfo &info);
  void on_hit(CacheSet *line, u32 pos, const MemoryAccessInfo &info);
  void on_arrive(CacheSet *line, u64 tag, const MemoryAccessInfo &info);
};

#endif
