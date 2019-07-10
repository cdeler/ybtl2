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

extern "C"
{
// the function returns ptr to be compatible with C
StackWalker *__attribute__((noinline)) recursive_call(int depth) {
  if (depth > 1) {
    return recursive_call(depth - 1);
  }

  return new StackWalker(StackWalker::unwind());
}
}

TEST_F(YbtlStackWalker, test_recursion) {
  int depth = 100;
  auto recursive_stack = recursive_call(depth);
  auto current_stack = StackWalker::unwind();

  ASSERT_EQ(current_stack.get_stack().size() + depth,
            recursive_stack->get_stack().size());

  auto &stack_data = recursive_stack->get_stack();

  auto actual_recursion_depth = count_if(
      stack_data.begin(),
      stack_data.end(),
      [](const stack_chunk_t &val) {
        return val.name_buffer == "recursive_call";
      });

  ASSERT_EQ(depth, actual_recursion_depth);

  delete recursive_stack;
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}