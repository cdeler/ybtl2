//
// Created by sergei_krotov on 7/4/19.
//

#include <string>
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

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}