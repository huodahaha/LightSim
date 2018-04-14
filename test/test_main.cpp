#include "gtest/gtest.h"
#include "memory_helper.h"

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



int main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
