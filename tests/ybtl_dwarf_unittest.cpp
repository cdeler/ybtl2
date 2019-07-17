//
// Created by sergei_krotov on 7/4/19.
//

#include <string>
#include <iostream>
#include <array>
#include <gtest/gtest.h>

#include "ybtl_dwarf.h"

using namespace std;
using namespace cdeler::ybtl2;

class TestUbtlDwarf : public testing::Test {
public:
  TestUbtlDwarf() {
    array<char, PATH_MAX> path{};
    const char *this_exe = "/proc/self/exe";

    readlink(this_exe, path.data(), PATH_MAX);
    executable_path = path.data();
  }

protected:
  string executable_path;
};

TEST_F(TestUbtlDwarf, it_works) {
  ExecutableDwarfData data{executable_path};
  ASSERT_TRUE(data.is_initialized());
}

static constexpr const size_t PREV_LINE = __LINE__;
extern void __attribute__((noinline, used)) b0e7d6d2_1535_4fe9_9a1c_60f538b7e564() {
  asm volatile ("nop");
}

TEST_F(TestUbtlDwarf, test_find_function_in_dwarf_data) {
  ExecutableDwarfData data{executable_path};
  data.load_dwarf_data();

  ASSERT_TRUE(data.is_dwarf_data_loaded());

  optional<function_data_t> a_function{nullopt};

  string looking_for = "b0e7d6d2_1535_4fe9_9a1c_60f538b7e564";

  for (auto &func: data) {
    if (looking_for == func.function_name) {
      a_function = make_optional(func);
      break;
    }
  }
  char this_file[] = __FILE__;

  ASSERT_TRUE(a_function.has_value());

  auto function = a_function.value();

  ASSERT_EQ(function.source_line, PREV_LINE + 1);
  ASSERT_STREQ(function.source_file_name.c_str(), basename(this_file));
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}