//
// Created by sergei_krotov on 7/9/19.
//

#include <string>
#include <iostream>
#include <array>
#include <gtest/gtest.h>

#include "ybtl_stack_walker.h"

using namespace std;
using namespace cdeler::ybtl2;

class YbtlStackWalker : public testing::Test {
};

TEST_F(YbtlStackWalker, it_works) {
  StackWalker stack = StackWalker::unwind();

  ASSERT_TRUE(!stack.get_stack().empty());
}

extern "C"
{
void nomangled_function() {
  StackWalker stack = StackWalker::unwind();

  auto last_frame = stack.get_stack().begin();

  ASSERT_TRUE(last_frame != stack.get_stack().end());

  ASSERT_STREQ(__func__, last_frame->name_buffer.c_str());
  ASSERT_EQ(reinterpret_cast<void *>(nomangled_function), last_frame->ip);
}
}

TEST_F(YbtlStackWalker, test_nomangled_function) {
  nomangled_function();
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}