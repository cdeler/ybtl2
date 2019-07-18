#include <utility>

//
// Created by sergei_krotov on 7/9/19.
//

#pragma once
#ifndef YBTL2_SRC_YBTL_STACK_WALKER_H
#define YBTL2_SRC_YBTL_STACK_WALKER_H

#include <string>
#include <vector>
#include <unwind.h>
#include <functional>

#include "ybtl_types.h"
#include "ybtl_dwarf.h"

namespace cdeler::ybtl2 {
struct stack_chunk_t {
  stack_chunk_t(void *addr, std::string val) noexcept
      : ip{addr}, name_buffer{std::move(val)} {};
  stack_chunk_t(stack_chunk_t &&st) noexcept
      : ip{st.ip}, name_buffer{std::move(st.name_buffer)} {};
  stack_chunk_t(const stack_chunk_t &) = default;

  void *ip;
  std::string name_buffer;
};

class StackWalker : private std::vector<function_data_t> {
private:
  using function_stack_t = std::vector<function_data_t>;
  using unwind_arg_data_t = std::tuple<function_stack_t &,
                                       ExecutableDwarfData &>;

  explicit StackWalker(function_stack_t &st) noexcept
      : function_stack_t{std::move(st)} {}

  static _Unwind_Reason_Code _trace_frame(struct _Unwind_Context *context, void *data);
  static std::tuple<void *, std::string> _get_symbol(const void *addr);

public:
  using function_stack_t::cbegin;
  using function_stack_t::cend;
  using function_stack_t::crbegin;
  using function_stack_t::crend;

  using function_stack_t::size;
  using function_stack_t::empty;

  static StackWalker unwind();
};
}

#endif //YBTL2_SRC_YBTL_STACK_WALKER_H
