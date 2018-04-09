#include "memory_hierarchy.h"

class CRRandomBlockFactory: public CacheBlockFactoryInterace {
 private:
  static CRRandomBlockFactory *_pinstance;
  CRRandomBlockFactory(){};

 public:
  ~CRRandomBlockFactory() {
    if (_pinstance) {
      delete _pinstance;
    }

  }
  static CacheBlockFactoryInterace* get_instance();
  CacheBlockBase* create(u64 addr, u64 tag, CacheSet *parent_set, u64 PC);
};

class CRRandomPolicy: public CRPolicyInterface {
 private:
  static CRRandomPolicy *_pinstance;
  CRRandomPolicy(CacheBlockFactoryInterace* factory): CRPolicyInterface(factory) {};

 public:
  ~CRRandomPolicy() {
    if (_pinstance) {
      delete _pinstance;
    }
    _pinstance = nullptr;
  }
  static CRPolicyInterface* get_instance();
  void on_hit(CacheSet *line, u32 pos, u64 addr, u64 PC);
  void on_arrive(CacheSet *line, u64 addr, u64 tag, u64 PC);
};
