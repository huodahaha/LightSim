#include "memory_helper.h"
#include "memory_hierarchy.h"
#include "cr_lru.h"
#include "cr_random.h"

#include <iostream>
#include <fstream>

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

  assert(len_of_binary(3) == 2);
  assert(len_of_binary(8) == 3);
  assert(len_of_binary(16) == 4);
  assert(len_of_binary(19) == 5);
}

void test_logger() {
  SIMLOG(SIM_INFO, "testmessage + %s\n", "msg");
  SIMLOG(SIM_WARNING, "testmessage + %s\n", "msg");
  SIMLOG(SIM_ERROR, "testmessage + %s\n", "msg");
}

void test_cache_unit() {
  u32 ways = 4;
  u32 blk_size = 128;
  u64 sets = 256;

  auto main__memory = MainMemory::get_instance();
  auto display = MemoryStats::get_instance();
  auto lru = CR_LRU_Policy::get_instance();
  auto random = CRRandomPolicy::get_instance();

  // test for lru + sequence
  {
    fprintf(stdout, "\n\ntest for LRU + sequence reference every 4 bytes\n");
    auto cache = new CacheUnit(ways, blk_size, sets, lru);

    for (u64 addr = 0; addr < (1 << 16); addr += 4) {
      u64 PC = 0;       // dummy PC
      auto ret = cache->try_access_memory(addr, PC);
      if (ret == false) {
        // cache miss
        ret = main__memory->try_access_memory(addr, PC);
        assert(ret);
        cache->on_memory_arrive(addr, PC);
      }
    }
    display->display(stdout);
    display->clear();
    delete cache;
  }

  // test for lru + sequence
  {
    fprintf(stdout, "\n\ntest for LRU + sequence every 16 bytes\n\n");
    auto cache = new CacheUnit(ways, blk_size, sets, lru);

    for (u64 addr = 0; addr < (1 << 16); addr += 16) {
      u64 PC = 0;       // dummy PC
      auto ret = cache->try_access_memory(addr, PC);
      if (ret == false) {
        // cache miss
        ret = main__memory->try_access_memory(addr, PC);
        assert(ret);
        cache->on_memory_arrive(addr, PC);
      }
    }
    display->display(stdout);
    display->clear();
    delete cache;
  }

  // test for random
  {
    fprintf(stdout, "\n\ntest for RANDOM + sequence every 16 bytes\n\n");
    auto cache = new CacheUnit(ways, blk_size, sets, random);

    for (u64 addr = 0; addr < (1 << 16); addr += 16) {
      u64 PC = 0;       // dummy PC
      auto ret = cache->try_access_memory(addr, PC);
      if (ret == false) {
        // cache miss
        ret = main__memory->try_access_memory(addr, PC);
        assert(ret);
        cache->on_memory_arrive(addr, PC);
      }
    }

    display->display(stdout);
    display->clear();
    delete cache;
  }

}

void test_trace() {
  u32 ways = 4;
  u32 blk_size = 64;
  u64 sets = 128;

  auto main__memory = MainMemory::get_instance();
  auto display = MemoryStats::get_instance();
  auto lru = CR_LRU_Policy::get_instance();
  auto random = CRRandomPolicy::get_instance();

  // test for lru + sequence
  {
    fprintf(stdout, "\n\ntest for LRU + test trace\n");
    auto cache = new CacheUnit(ways, blk_size, sets, lru);

    const char *trace = "test_trace";
    // test for test_trace
    ifstream infile(trace, fstream::in);

    u64 addr;
    u64 PC = 0;       // dummy PC
    while (!infile.eof()) {
      infile >> hex >> addr;
      auto ret = cache->try_access_memory(addr, PC);
      if (ret == false) {
        // cache miss
        ret = main__memory->try_access_memory(addr, PC);
        assert(ret);
        cache->on_memory_arrive(addr, PC);
      }
    }

    display->display(stdout);
    display->clear();
    delete cache;
  }

  // test for random + sequence
  {
    fprintf(stdout, "\n\ntest for random + test trace\n");
    auto cache = new CacheUnit(ways, blk_size, sets, random);

    const char *trace = "test_trace";
    // test for test_trace
    ifstream infile(trace, fstream::in);

    u64 addr;
    u64 PC = 0;       // dummy PC
    while (!infile.eof()) {
      infile >> hex >> addr;
      auto ret = cache->try_access_memory(addr, PC);
      if (ret == false) {
        // cache miss
        ret = main__memory->try_access_memory(addr, PC);
        assert(ret);
        cache->on_memory_arrive(addr, PC);
      }
    }

    display->display(stdout);
    display->clear();
    delete cache;
  }

}


int main() {
  test_valid_addr();
  test_logger();
  test_cache_unit();
  test_trace();
}
