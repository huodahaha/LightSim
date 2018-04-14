#include "gtest/gtest.h"
#include "memory_helper.h"
#include "trace_loader.h"
#include <iostream>

TEST(CheckBinaryFunctions, check_addr_valid) {
  EXPECT_EQ(check_addr_valid(1), true);
  EXPECT_EQ(check_addr_valid(200), true);
  EXPECT_EQ(check_addr_valid(255), true);
  EXPECT_EQ(check_addr_valid(~0), true);
}


TEST(CheckBinaryFunctions, power_of_two) {
  EXPECT_EQ(is_power_of_two(0), true);
  EXPECT_EQ(is_power_of_two(1), true);
  EXPECT_EQ(is_power_of_two(2), true);
  EXPECT_EQ(is_power_of_two(4), true);
  EXPECT_EQ(is_power_of_two(9), false);
  EXPECT_EQ(is_power_of_two(17), false);
}

TEST(CheckBinaryFunctions, len_of_binary) {
  EXPECT_EQ(len_of_binary(3), 2);
  EXPECT_EQ(len_of_binary(8), 3);
  EXPECT_EQ(len_of_binary(16), 4);
  EXPECT_EQ(len_of_binary(19), 5);
}


class TestTraceLoader : public ::testing::Test {
 protected:
  TraceLoader* trace_loader;
  void print_trace(const TraceFormat& trace) const {
    printf("pc: %016llX\n", trace.pc);
    printf("opcode: %08X :", trace.opcode);
    printf("%s\n", trace.opcode_string);
    printf("thread_id: %u\n", trace.thread_id);
    printf("is_branch: %u\n", trace.is_branch);
    printf("branch_taken: %u\n", trace.branch_taken);
  }

  TestTraceLoader() {
    std::cout << "Please enter a path to a single thread trace file" << std::endl;
    string filename;
    getline(std::cin, filename);
    trace_loader = new TraceLoader(filename);
  }

  ~TestTraceLoader() {
    delete trace_loader;
  }

};


TEST_F(TestTraceLoader, get_traces) {
  int count = 0;
  unsigned char tid;
  unsigned char last_tid;
  bool initialized = false;
  while (!trace_loader->is_end()) {
    auto new_trace = trace_loader->next_instruction();
    if (new_trace.first) {
      if (count++ < 2000) {
        // printing the first 20 for check
        print_trace(new_trace.second);
      }
      tid = new_trace.second.thread_id;
      if (initialized) {
        ASSERT_EQ(tid, last_tid);
      }
      last_tid = tid;
      initialized = true;
    }
  }
  EXPECT_EQ(initialized, true);
}

int main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
