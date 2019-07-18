//
// Created by sergei_krotov on 7/9/19.
//

#include <string>
#include <iostream>
#include <array>
#include <gtest/gtest.h>
#include <functional>

#include "ybtl_stack_walker.h"

using namespace std;
using namespace cdeler::ybtl2;

class YbtlStackWalker : public testing::Test {
};

TEST_F(YbtlStackWalker, it_works) {
  StackWalker stack = StackWalker::unwind();

  ASSERT_TRUE(!stack.empty());
}

TEST_F(YbtlStackWalker, test_deepest_symbol_is__start) {
  StackWalker stack = StackWalker::unwind();

  auto last_symbol = stack.crbegin();

  ASSERT_STREQ("_start", last_symbol->name_buffer.c_str());
}

extern "C"
{
void nomangled_function() {
  StackWalker stack = StackWalker::unwind();

  auto last_frame = stack.cbegin();

  ASSERT_TRUE(last_frame != stack.cend());

  ASSERT_STREQ(__func__, last_frame->name_buffer.c_str());
  ASSERT_EQ(reinterpret_cast<void *>(nomangled_function), last_frame->ip);
}
}

TEST_F(YbtlStackWalker, test_called_function_exists_in_unwind_data) {
  nomangled_function();
}

extern "C"
{
// the function returns ptr to be compatible with C
tuple<StackWalker, string> *recursive_function(volatile int depth) {
  if (depth > 1) {
    asm volatile ("nop");
    return recursive_function(depth - 1);
  }

  return new tuple<StackWalker, string>(StackWalker::unwind(), __func__);
}
}

TEST_F(YbtlStackWalker, test_recursive_function_count_in_unwind_data) {
  int depth = 100;

  unique_ptr<tuple<StackWalker, string>> recursive_stack{recursive_function(depth)};

  auto[stack_walker, function_name] = *recursive_stack;

  auto current_stack = StackWalker::unwind();

  ASSERT_EQ(current_stack.size() + depth,
            stack_walker.size());

  auto actual_recursion_depth = count_if(
      stack_walker.cbegin(),
      stack_walker.cend(),
      [name = std::ref(function_name)](const stack_chunk_t &val) {
        return val.name_buffer == name.get();
      });

  ASSERT_EQ(depth, actual_recursion_depth);
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}