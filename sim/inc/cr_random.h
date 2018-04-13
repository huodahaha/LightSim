#include "memory_hierarchy.h"

class CRRandomBlockFactory: public CacheBlockFactoryInterace {
 private:
  static CRRandomBlockFactory _instance;

 public:
  CRRandomBlockFactory(u32 policy_id) : CacheBlockFactoryInterace(policy_id) {};
  CacheBlockFactoryInterace* get_instance();
  CacheBlockBase* create(u64 addr, u64 tag, CacheSet *parent_set, u64 PC);
};

class CRRandomPolicy: public CRPolicyInterface {
 private:
  static CRRandomPolicy _instance;

 public:
  CRRandomPolicy(u32 policy_id): CRRandomPolicy(policy_id) {};
  CRPolicyInterface* get_instance();
  void on_hit(CacheSet *line, u32 pos, u64 addr, u64 PC);
  void on_miss(CacheSet *line, u64 addr, u64 tag, u64 PC);
};
