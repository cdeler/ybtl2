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

class StackWalker {
public:
  StackWalker(StackWalker &&sw) noexcept
      : m_stack{std::move(sw.m_stack)} {};

  StackWalker(const StackWalker &sw) = default;

  const std::vector<stack_chunk_t> &get_stack() const noexcept {
    return m_stack;
  }

  static StackWalker unwind();
private:
  explicit StackWalker(std::vector<stack_chunk_t> &st) noexcept
      : m_stack{std::move(st)} {}

  static _Unwind_Reason_Code _trace_frame(struct _Unwind_Context *context, void *data);
  static std::tuple<void *, std::string> _get_symbol(const void *addr);

  std::vector<stack_chunk_t> m_stack;
};
}

#endif //YBTL2_SRC_YBTL_STACK_WALKER_H
