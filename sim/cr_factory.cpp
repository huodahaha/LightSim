#include "cr_factory.h"
#include "cr_lru.h"
#include "cr_random.h"
#include "cr_lip.h"

PolicyFactory::~PolicyFactory() {
  for (auto p: _policies) {
    assert(p != NULL);
    delete p;
  }
}

CRPolicyInterface* PolicyFactory::get_policy(CR_POLICY policy_type) {
  auto iter = _shared_policies.find(policy_type);
  if (iter != _shared_policies.end())
    return iter->second;

  auto ret = create_policy(policy_type);
  _shared_policies[policy_type] = ret;
  return ret;
}

CRPolicyInterface* PolicyFactory::create_policy(CR_POLICY policy_type) {
  CRPolicyInterface* ret = NULL;
  CacheBlockFactoryInterace *factory = NULL;
  switch (policy_type) {
    case LRU_POLICY:
      factory = new CR_LRU_BlockFactory();
      ret = new CR_LRU_Policy(factory);
      break;

    case RANDOM_POLICY:
      factory = new CRRandomBlockFactory();
      ret = new CRRandomPolicy(factory);
      break;

    default:
      assert(0);
      break;
  }
  _policies.push_back(ret);
  return ret;
}
