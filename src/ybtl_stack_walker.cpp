//
// Created by sergei_krotov on 7/9/19.
//

#include <dlfcn.h>
#include <tuple>
#include <iostream>
#include <memory.h>
#include "ybtl_stack_walker.h"

using namespace std;
using namespace cdeler::ybtl2;

StackWalker StackWalker::unwind() {
  vector<stack_chunk_t> stack;

  _Unwind_Backtrace(_trace_frame, &stack);

  return StackWalker{stack};
}

_Unwind_Reason_Code StackWalker::_trace_frame(struct _Unwind_Context *context, void *data) {
  auto storage = reinterpret_cast<vector<stack_chunk_t> *>(data);
  _Unwind_Reason_Code rc = _URC_NO_REASON;

  auto addr = reinterpret_cast<void *>(_Unwind_GetIP(context));

  if (addr != nullptr) {
    auto[function_base, function_name] = _get_symbol(addr);

    if (function_base != StackWalker::unwind) {
      storage->emplace_back(function_base, function_name);
    }
  }

  return rc;
}

tuple<void *, string> StackWalker::_get_symbol(const void *addr) {
  Dl_info info;
  int res = dladdr(addr, &info);
  string result_name;

  if (res) {
    const char *fname = info.dli_sname;

    if (fname)
      result_name = fname;
    else
      result_name = "(nullptr)";
  } else {
    result_name = "(ERROR)";
  }

  return make_tuple(info.dli_saddr, result_name);
}
