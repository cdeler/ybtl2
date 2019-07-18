//
// Created by sergei_krotov on 7/9/19.
//

#include <dlfcn.h>
#include <tuple>
#include <iostream>
#include <memory.h>
#include <functional>

#include "ybtl_stack_walker.h"
#include "ybtl_dwarf.h"

using namespace std;
using namespace cdeler::ybtl2;

StackWalker StackWalker::unwind() {
  vector<function_data_t> stack;

  ExecutableDwarfData dwarf_data;

  dwarf_data.load_dwarf_data();

  unwind_arg_data_t unwind_param = make_tuple(std::ref(stack), std::ref(dwarf_data));

  _Unwind_Backtrace(_trace_frame, &unwind_param);

  return StackWalker{stack};
}

_Unwind_Reason_Code StackWalker::_trace_frame(struct _Unwind_Context *context, void *data) {
  auto unwind_data = reinterpret_cast<unwind_arg_data_t *>(data);

  auto &[unwind_storage, dwarf_data] = *unwind_data;

  auto frame_ip_address = reinterpret_cast<void *>(_Unwind_GetIP(context));

  if (frame_ip_address != nullptr) {
    auto[function_base, function_name] = _get_symbol(frame_ip_address);

    if (function_base != StackWalker::unwind) {
      if (dwarf_data.is_dwarf_data_loaded()) {
        auto dwarf_record = dwarf_data.find(function_name);
        if (dwarf_record != dwarf_data.cend()) {
          unwind_storage.emplace_back(function_name,
                                      dwarf_record->second.source_file_name,
                                      dwarf_record->second.source_line);
        } else {
          unwind_storage.emplace_back(function_name, "", 0);
        }
      } else {
        unwind_storage.emplace_back(function_name, "", 0);
      }
    }
  }

  return _URC_NO_REASON;
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
