#include "memory_helper.h"
#include "memory_hierarchy.h"
#include "trace_loader.h"
#include "cfg_loader.h"

#include <iostream>
#include <fstream>
#include <time.h>
#include <cmath>
#include <cstring>

void test_valid_addr() {
  assert(check_addr_valid(1));
  assert(check_addr_valid(200));
  assert(check_addr_valid(255));
  assert(check_addr_valid(~0));

  assert(is_power_of_two(0));
  assert(is_power_of_two(1));
  assert(is_power_of_two(2));
  assert(is_power_of_two(4));
  assert(is_power_of_two(9) == false);
  assert(is_power_of_two(17) == false);

  assert(len_of_binary(1) == 0);
  assert(len_of_binary(8) == 3);
  assert(len_of_binary(16) == 4);
}

void test_logger() {
  SIMLOG(SIM_INFO, "testmessage + %s\n", "msg");
  SIMLOG(SIM_WARNING, "testmessage + %s\n", "msg");
  SIMLOG(SIM_ERROR, "testmessage + %s\n", "msg");
}

void test_event_engine() {
  class TestHandler: public EventHandler {
   private:
    u64 t;
    u32 ttl;

   protected:
    bool validate(EventType t) {
      (void)t;
      return true;
    }

    void proc(u64 tick, EventDataBase* data, EventType type) {
      assert(tick == t);
      ttl--;
      if (ttl == 0)
        return;
      EventEngine *evnet_queue = EventEngineObj::get_instance();
      EventDataBase *d = new EventDataBase();
      Event *e = new Event(MemoryOnArrive, this, d);
      evnet_queue->register_after_now(e, 5, 5);
      t += 5;
    }

   public:
    TestHandler() : t(0), ttl(100) {};
  };

  EventEngine* engine = new EventEngine();
  EventHandler* handler = new TestHandler();
  EventDataBase *d = new EventDataBase();
  Event *e = new Event(MemoryOnArrive, handler, d);
  engine->register_after_now(e, 0, 0);

  while (true) {
    auto ret = engine->loop();
    if (ret == 0) {
      break;
    } 
  }
}

void test_lru_set() {
  u32 ways = 8;
  u32 blk_size = 128;
  u32 sets = 32;
  bool ret;

  auto factory = PolicyFactoryObj::get_instance();
  CRPolicyInterface* lru = factory->create_policy(LRU_POLICY);
  CacheSet *line = new CacheSet(ways, blk_size, sets, lru);

  u64 addr = 1 << 16;
  MemoryAccessInfo info(addr, 0);
  ret = line->try_access_memory(info);
  line->on_memory_arrive(info);
  assert(ret == false);

  for (u64 shift = 0; shift < 128; shift += 4) {
    MemoryAccessInfo info(addr + shift, 0);
    ret = line->try_access_memory(info);
    assert(ret);
  }

  vector<u64> addrs;
  for (u64 idx = 0; idx < ways; idx++) {
    u64 shift = idx << 40;
    addrs.push_back(addr + shift);
    MemoryAccessInfo info(addr + shift, 0);
    line->on_memory_arrive(info);
  }

  auto blocks = line->get_all_blocks();
  for (auto block : blocks) {
    (void)block;
    assert(block != NULL);
  }

  for (u32 cnt = 0; cnt < 20; cnt++) {
    u64 addr = addrs[(rand() % ways)];
    MemoryAccessInfo info(addr, 0);
    auto old_blocks = line->get_all_blocks();
    ret = line->try_access_memory(info);
    assert(ret == true);

    // check LRU
    auto new_blocks = line->get_all_blocks();
    for (u32 idx = 0; idx < ways ; idx++) {
      if (old_blocks[idx]->get_addr() == addr) {
        for (idx = idx + 1; idx < ways; idx++) {
          assert(new_blocks[idx]->get_addr() == old_blocks[idx]->get_addr());
        }
        break;
      }
      assert(new_blocks[idx + 1]->get_addr() == old_blocks[idx]->get_addr());
    }
  }
}

void test_random_set() {
  u32 ways = 4;
  u32 blk_size = 128;
  u32 sets = 32;

  auto factory = PolicyFactoryObj::get_instance();
  CRPolicyInterface* lru = factory->create_policy(RANDOM_POLICY);
  CacheSet *line = new CacheSet(ways, blk_size, sets, lru);

  u64 addr = 1 << 16;
  for (u64 shift = 0; shift < blk_size * ways; shift += blk_size) {
    MemoryAccessInfo info(addr + shift, 0);
    line->on_memory_arrive(info);
  }

  auto old_blocks = line->get_all_blocks();
  for (auto block : old_blocks) {
    (void)block;
    assert(block != NULL);
  }

  // random access
  for (u32 cnt = 0; cnt < 800 * ways; cnt++) {
    MemoryAccessInfo info(rand(), 0);
    line->on_memory_arrive(info);
  }
  //auto new_blocks = line->get_all_blocks();
  //for (u32 idx = 0; idx < ways; idx++) {
    //assert(old_blocks[idx]->get_addr() != new_blocks[idx]->get_addr());
  //}
}

bool prefix(const char * str, const char * prefix) {
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

void test_trace_loader() {
  TraceLoader loader("../traces/ls_trace.trace.gz");
  TraceFormat trace;
  TraceFormat last_trace;
  loader.next_instruction(last_trace);
  while (loader.next_instruction(trace)) {
    if (! (last_trace.is_branch && last_trace.branch_taken) &&
           last_trace.thread_id == trace.thread_id) {
      if ((trace.pc - last_trace.pc) >= 16) {
        assert(prefix(last_trace.opcode_string, "RET") ||
               prefix(last_trace.opcode_string, "CALL"));
      }
    }
    last_trace = trace;
  }

}

void test_cfg_loader() {
  auto loader = CfgLoaderObj::get_instance();
  loader->parse("../cfg/cfg.json");

  auto nodes = loader->get_nodes();
  vector<BaseNodeCfg *> cpus;
  vector<BaseNodeCfg *> memory;

  for (auto &entry: nodes) {
    BaseNodeCfg *node = entry.second;
    if (node->type == CpuNode) {
      cpus.push_back(node);
    }
    else if (node->type == MemoryNode){
      memory.push_back(node);
    }
  }

  assert(cpus.size() > 0);
  assert(memory.size() == 1);

  for (auto cpu: cpus) {
    auto p = cpu;
    while (p->next_node) {
      p = p->next_node;
    }
    assert(p == memory[0]);
  }
}

void test_memory_pipeline() {
  vector<u64> mock_trace;
  u64 addr = 0;
  for (int i = 0; i < 8; i++) {
    mock_trace.push_back(addr);
    addr += 32;
  }

  MemoryConfig main_memory_cfg(32, 100);
  //MemoryConfig cache_memory_cfg(16, 10, 8, 128, 128, LRU_POLICY);

  //CacheUnit* cache = new CacheUnit(cache_memory_cfg);
  MainMemory* memory = new MainMemory(main_memory_cfg);
  memory->attach_tag("Main Memory");
  CpuConnector* cpu = new CpuConnector(mock_trace);
  cpu->attach_tag("CPU Connector");

  // assamble
  cpu->set_next(memory);
  memory->add_prev(cpu);

  cpu->issue_memory_access();

  EventEngine *evnet_queue = EventEngineObj::get_instance();
  while (true) {
    auto ret = evnet_queue->loop();
    if (ret == 0)
      break;
  }
}

int main() {
  srand(time(NULL));
  test_valid_addr();
  //test_logger();
  test_event_engine();
  test_lru_set();
  test_random_set();
  test_trace_loader();
  test_cfg_loader();

  test_memory_pipeline();
}
