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

  ASSERT_NE(last_symbol, stack.crend());
  ASSERT_STREQ("_start", last_symbol->function_name.c_str());
}

extern "C"
{
void nomangled_function() {
  StackWalker stack = StackWalker::unwind();

  auto last_frame = stack.cbegin();

  ASSERT_TRUE(last_frame != stack.cend());

  ASSERT_STREQ(__func__, last_frame->function_name.c_str());
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
      [name = std::ref(function_name)](const function_data_t &val) {
        return val.function_name == name.get();
      });

  ASSERT_EQ(depth, actual_recursion_depth);
}

extern "C" {
void *test_function() {
  asm volatile ("nop");
  char file_name[] = __FILE__;
  return new tuple<StackWalker, string, int>(StackWalker::unwind(),
                                             string{basename(file_name)},
                                             __LINE__ - 5);
}
}

TEST_F(YbtlStackWalker, test_for_line_no_and_source_file) {
  auto call_result_ptr = reinterpret_cast<tuple<StackWalker, string, int> *>(test_function());
  unique_ptr<tuple<StackWalker, string, int>> call_result(call_result_ptr);

  auto &[unwind_data, file_name, line_no] = *call_result;

  auto function_handle_in_stack = unwind_data.cbegin();

  ASSERT_NE(function_handle_in_stack, unwind_data.cend());

  ASSERT_STREQ(function_handle_in_stack->source_file_name.c_str(), file_name.c_str());
  ASSERT_EQ(function_handle_in_stack->source_line, line_no);
}

StackWalker mangled_test_function() {
  return StackWalker::unwind();
}

TEST_F(YbtlStackWalker, test_for_mangled_function) {
  auto unwind_result = mangled_test_function();

  auto last_function = unwind_result.cbegin();

  ASSERT_NE(unwind_result.cend(), last_function);

  auto last_function_address = reinterpret_cast<intptr_t>(&mangled_test_function);

  ASSERT_EQ(last_function_address, last_function->function_address);
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}