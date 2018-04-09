#ifndef CR_FACTORY_H
#define CR_FACTORY_H

#include "inc_all.h"
#include "memory_hierarchy.h"

enum CR_POLICY {
  LRU_POLICY,
  RANDOM_POLICY,

  POLICY_CNT
};

class PolicyFactory {
 private:
  map<CR_POLICY, CRPolicyInterface*>      _shared_policies;
  vector<CRPolicyInterface*>              _policies;

 public:
  PolicyFactory() {};
  ~PolicyFactory();

  CRPolicyInterface* get_policy(CR_POLICY policy_type);

  // when policy use private data
  CRPolicyInterface* create_policy(CR_POLICY policy_type);
};

typedef Singleton<PolicyFactory> PolicyFactoryObj;

#endif
