#include "memory_helper.h"
#include "memory_hierarchy.h"

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

  assert(len_of_binary(0) == 0);
  assert(len_of_binary(3) == 2);
  assert(len_of_binary(8) == 4);
  assert(len_of_binary(16) == 5);
  assert(len_of_binary(19) == 5);
}

int main() {
  test_valid_addr();
}
