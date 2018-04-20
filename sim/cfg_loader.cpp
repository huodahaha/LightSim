#include "cfg_loader.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

static void check_key(Value &value, const char *key, const char *node_name) {
  if (!value.HasMember(key)) {
    fprintf(stderr, "<%s> has to provide %s filed\n", node_name, key);
    exit(1);
  }
}

static BaseNodeCfg* parse_node(Value &node) {
  BaseNodeCfg *node_cfg = NULL;

  check_key(node, "type", "unknown node");
  string type = node["type"].GetString();

  check_key(node, "name", "unknown node");
  string name = node["name"].GetString();

  if (type == "cpu") {
    node_cfg = new CpuNodeCfg(CpuNode, name);
  }

  else if (type == "cache"){
    check_key(node, "latency", name.c_str());
    int latency = node["latency"].GetInt();
    check_key(node, "blocksize", name.c_str());
    int blocksize = node["blocksize"].GetInt();
    check_key(node, "assoc", name.c_str());
    int assoc = node["assoc"].GetInt();
    check_key(node, "sets", name.c_str());
    int sets = node["sets"].GetInt();
    check_key(node, "policy", name.c_str());
    string policy = node["policy"].GetString();
    node_cfg = new CacheNodeCfg(CacheNode, name, latency, blocksize, assoc,
                                sets, policy);
  }

  else if (type == "memory") {
    check_key(node, "latency", name.c_str());
    int latency = node["latency"].GetInt();
    node_cfg = new MemoryNodeCfg(MemoryNode, name, latency);
  }

  else {
    fprintf(stderr, "unknown type name %s\n", type.c_str());
  }
  return node_cfg;
}

static NetworkCfg* parse_network(Value &node) {
  NetworkCfg *network_cfg = NULL;

  check_key(node, "name", "unknown node");
  string name = node["name"].GetString();
  check_key(node, "input", name.c_str());
  string input = node["input"].GetString();
  check_key(node, "output", name.c_str());
  string output = node["output"].GetString();

  network_cfg = new NetworkCfg(name, input, output);

  return network_cfg;
}

static void parse_nodes(Value &nodes, map<string, BaseNodeCfg*> &node_map) {
  // there should be an unique main memory node
  vector<BaseNodeCfg*> main_memory;

  for (auto& v : nodes.GetArray()) {
    BaseNodeCfg *node_cfg = parse_node(v);
    assert(node_cfg);

    if (node_cfg->type == MemoryNode) {
      main_memory.push_back(node_cfg);
    }

    auto iter = node_map.find(node_cfg->name);
    if (iter != node_map.end()) {
      fprintf(stderr, "duplcate node name: \"%s\"\n", node_cfg->name.c_str());
    }
    else {
      node_map[node_cfg->name] = node_cfg;
    }
  }

  if (main_memory.size() != 1) {
    fprintf(stderr, "there should be an unique main memory node"); 
    exit(1);
  }
}

static void parse_networks(Value &nodes, map<string, NetworkCfg*> &network_map) {
  for (auto& v : nodes.GetArray()) {
    NetworkCfg *network_cfg = parse_network(v);
    assert(network_cfg);

    auto iter = network_map.find(network_cfg->name);
    if (iter != network_map.end()) {
      fprintf(stderr, "duplcate network name: \"%s\"\n", network_cfg->name.c_str());
    }
    else {
      network_map[network_cfg->name] = network_cfg;
    }
  }
}

static void check_name_exist(map<string, BaseNodeCfg*> &node_map, string &node_name, string &network_name) {
    auto iter = node_map.find(node_name);
    if (iter == node_map.end()) {
      fprintf(stderr, "<network: %s> cannot find node with name <%s>\n",
              network_name.c_str(), node_name.c_str());
      exit(1);
    } 
}

static void assamble(map<string, NetworkCfg*> &network_map, map<string, BaseNodeCfg*> &node_map) {
  for (auto entry: network_map) {
    auto network = entry.second;

    check_name_exist(node_map, network->input, network->name);
    check_name_exist(node_map, network->output, network->name);

    node_map[network->input]->next_node = node_map[network->output];
  }
}

// after assamling ,the node wuold form a tree with memory node as root and 
// cpu node as leaf
static bool check_node(BaseNodeCfg *node) {
  if (node == NULL) {
    fprintf(stderr, "node <%s> don't have a proceeding node,", node->name.c_str());
    fprintf(stderr, "every node should end up with a memory node\n");
    return false;
  }
  else if (node->type == MemoryNode) {
    return (node->next_node == NULL);
  }
  else {
    return check_node(node->next_node);
  } 
}

static bool check_tree(map<string, BaseNodeCfg*> node_map) {
  for (auto entry: node_map) {
    auto node = entry.second;
    if (node->type == CpuNode) {
      auto ret = check_node(node);
      if (ret != true) {
        return false;
      }
    }
  }
  return true;
}

void check_documents(Document &d, const char *key) {
  if (!d.HasMember(key)) {
    fprintf(stderr, "document should provide <%s> field\n", key);
    exit(1);
  }
}

template<typename T1, typename T2>
void delete_map(map<T1, T2> m) {
  for (auto entry: m) {
    auto v = entry.second;
    delete v;
  }
}  

void SimCfgLoader::delete_nodes() {
  delete_map<>(_networks_map);
  delete_map<>(_nodes_map);
}

SimCfgLoader::~SimCfgLoader() {
  delete_nodes();
}

void SimCfgLoader::parse(string filename) {
  FILE* fp = fopen(filename.c_str(), "rb"); 
  char readBuffer[65536];
  FileReadStream is(fp, readBuffer, sizeof(readBuffer));
  Document d;
  d.ParseStream(is);
  fclose(fp);

  if (!d.IsObject()) {
    fprintf(stderr, "%s parse failure, may due to bad format\n", filename.c_str());
    exit(1);
  }

  check_documents(d, "nodes");
  check_documents(d, "networks");

  Value& nodes = d["nodes"];
  Value& networks = d["networks"];

  if (!nodes.IsArray()) {
    fprintf(stderr, "\"nodes\" cfg should be an array\n");
    exit(1);
  }

  if (!networks.IsArray()) {
    fprintf(stderr, "\"networks\" cfg should be an array\n");
    exit(1);
  }

  delete_nodes();
  parse_nodes(nodes, _nodes_map);
  parse_networks(networks, _networks_map);

  assamble(_networks_map, _nodes_map);
  assert(check_tree(_nodes_map));
}
