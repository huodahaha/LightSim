#include "memory_hierarchy.h"

class CR_LRU_BlockFactory: public CacheBlockFactoryInterace {
 private:
  static CR_LRU_BlockFactory _instance;

 public:
  CR_LRU_BlockFactory(u32 policy_id) : CacheBlockFactoryInterace(policy_id) {};
  CacheBlockFactoryInterace* get_instance();
  CacheBlockBase* create(u64 addr, u64 tag, CacheSet *parent_set, u64 PC);
};

class CR_LRU_Policy: public CRPolicyInterface {
 private:
  static CR_LRU_Policy _instance;

 public:
  CR_LRU_Policy(u32 policy_id): CR_LRU_Policy(policy_id) {};
  CRPolicyInterface* get_instance();
  void on_hit(CacheSet *line, u32 pos, u64 addr, u64 PC);
  void on_miss(CacheSet *line, u64 addr, u64 tag, u64 PC);
};
