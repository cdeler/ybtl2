//
// Created by sergei_krotov on 7/4/19.
//
#include <vector>
#include <libgen.h>

#include "ybtl_dwarf.h"

using namespace std;
using namespace cdeler::ybtl2;

bool ExecutableDwarfData::is_initialized() const noexcept {
  return dwarf_.dwarf != nullptr && elf_.elf != nullptr && fd_ >= 0;
}

ExecutableDwarfData::~ExecutableDwarfData() noexcept {
  if (fd_opened_internally_ && fd_ >= 0) {
    close(fd_);
  }
}

bool ExecutableDwarfData::load_dwarf_data() {
  if (!is_dwarf_data_loaded_) {
    lock_guard<mutex> lock{data_load_mutex_};

    if (!is_dwarf_data_loaded_) {
      _read_dwarf_data();
      is_dwarf_data_loaded_ = !functions_.empty();
      return is_dwarf_data_loaded();
    }
  }
  return false;
}

void ExecutableDwarfData::_read_dwarf_data() {
  Dwarf_Off offset = 0U, last_offset = 0U;
  size_t header_size = 0U;

  while (dwarf_nextcu(dwarf_.dwarf, offset, &offset, &header_size, nullptr, nullptr, nullptr) == 0) {
    Dwarf_Die child_die, control_unit_die;
    if (dwarf_offdie(dwarf_.dwarf, last_offset + header_size, &control_unit_die) == nullptr)
      break;

    last_offset = offset;

    if (dwarf_child(&control_unit_die, &child_die) != 0)
      continue;

    do {
      switch (dwarf_tag(&child_die)) {
      case DW_TAG_entry_point:
      case DW_TAG_inlined_subroutine:
      case DW_TAG_subprogram:
        function_data_t new_function{&child_die};

        functions_.emplace(new_function.function_address, new_function);
        break;
      }
    } while (dwarf_siblingof(&child_die, &child_die) == 0);
  }
}

