#include "memory_hierarchy.h"

class CR_LRU_BlockFactory: public CacheBlockFactoryInterace {
 private:
  static CR_LRU_BlockFactory *_pinstance;
  CR_LRU_BlockFactory() {};

 public:
  static CacheBlockFactoryInterace* get_instance();
  CacheBlockBase* create(u64 addr, u64 tag, CacheSet *parent_set, u64 PC);
};

class CR_LRU_Policy: public CRPolicyInterface {
 private:
  static CR_LRU_Policy *_pinstance;
  CR_LRU_Policy(CacheBlockFactoryInterace* factory): CRPolicyInterface(factory) {};

 public:
  static CRPolicyInterface* get_instance();
  void on_hit(CacheSet *line, u32 pos, u64 addr, u64 PC);
  void on_arrive(CacheSet *line, u64 addr, u64 tag, u64 PC);
};
