//
// Created by sergei_krotov on 7/4/19.
//

#include <string>
#include <iostream>
#include <gtest/gtest.h>

#include "ybtl_dwarf.h"

using namespace std;
using namespace cdeler::ybtl2;

class TestUbtlDwarf : public testing::Test {
public:
  TestUbtlDwarf() {
    char path[PATH_MAX];
    const char *this_exe = "/proc/self/exe";
    memset(path, 0, sizeof(path)); // readlink doesn't make null term string

    readlink(this_exe, path, PATH_MAX);
    executable_path = path;
  }

protected:

  string executable_path;
};

TEST_F(TestUbtlDwarf, it_works) {
  ExecutableDwarfData data{executable_path};
  ASSERT_TRUE(data.is_initialized());
}

extern void __attribute__((noinline)) test_function() {
  asm volatile ("nop");
}

TEST_F(TestUbtlDwarf, get_function_list) {
  ExecutableDwarfData data{executable_path};
  data.load_dwarf_data();

  ASSERT_TRUE(data.is_dwarf_data_loaded());

  auto functions = data.get_function_data();

  bool is_test_function_found = false;
  string looking_for = "test_function";

  for (auto &func: functions) {
    if (looking_for == func.function_name) {
      is_test_function_found = true;
      break;
    }
  }

  ASSERT_TRUE(is_test_function_found);
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}