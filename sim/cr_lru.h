#include "memory_hierarchy.h"

class CR_LRU_BlockFactory: public CacheBlockFactoryInterace {
 private:
  static CR_LRU_BlockFactory *_pinstance;
  CR_LRU_BlockFactory() {};

 public:
  ~CR_LRU_BlockFactory() {
    if (_pinstance) delete _pinstance;
    _pinstance = nullptr;
  }

  static CacheBlockFactoryInterace* get_instance();
  CacheBlockBase* create(u64 addr, u64 tag, CacheSet *parent_set, u64 PC);
};

class CR_LRU_Policy: public CRPolicyInterface {
 private:
  static CR_LRU_Policy *_pinstance;
  CR_LRU_Policy(CacheBlockFactoryInterace* factory): CRPolicyInterface(factory) {};

 public:
  ~CR_LRU_Policy() {
    if (_pinstance) delete _pinstance;
    _pinstance = nullptr;
  }

  static CRPolicyInterface* get_instance();
  void on_hit(CacheSet *line, u32 pos, u64 addr, u64 PC);
  void on_arrive(CacheSet *line, u64 addr, u64 tag, u64 PC);
};
