#ifndef CFG_LOADER_H
#define CFG_LOADER_H

#include "inc_all.h"

struct BaseNodeCfg;
struct CacheNodeCfg;
struct MemoryNodeCfg;
struct NetworkCfg;

using namespace std;

enum CfgNodeType {
  CpuNode,
  CacheNode,
  MemoryNode
};

struct BaseNodeCfg {
  CfgNodeType       type;
  string            name;
  BaseNodeCfg*      next_node;

  BaseNodeCfg(CfgNodeType type_, string name_) : type(type_), name(name_), next_node(NULL){};
};

struct NetworkCfg {
  string            name;
  string            input;
  string            output;

  NetworkCfg(string name_, string input_, string output_) 
      : name(name_), input(input_), output(output_){};
};

struct CpuNodeCfg : public BaseNodeCfg {
  CpuNodeCfg(CfgNodeType type_, string name_) : BaseNodeCfg(type_, name_){};
};

struct CacheNodeCfg: public BaseNodeCfg {
  int               latency;
  int               blocksize;
  int               assoc;
  int               sets;
  string            cr_policy;

  CacheNodeCfg(CfgNodeType type_, string name_, int latency_, int blocksize_,
               int assoc_, int sets_, string policy) : BaseNodeCfg(type_, name_),
               latency(latency_), blocksize(blocksize_), assoc(assoc_), 
               sets(sets_), cr_policy(policy) {};
};

struct MemoryNodeCfg: public BaseNodeCfg {
  int               latency;

  MemoryNodeCfg(CfgNodeType type_, string name_, int latency_) : 
              BaseNodeCfg(type_, name_), latency(latency_){};
};

class SimCfgLoader {
 private:
  map<string, BaseNodeCfg*>   _nodes_map;
  map<string, NetworkCfg*>    _networks_map;

  void delete_nodes();

 public:
  SimCfgLoader() {};
  ~SimCfgLoader();

  inline const map<string, BaseNodeCfg*> &get_nodes() {
    return _nodes_map;
  }

  void parse(string filename);
};

typedef Singleton<SimCfgLoader> CfgLoaderObj;

#endif
