//
// Created by sergei_krotov on 7/4/19.
//
#include <vector>

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
      is_dwarf_data_loaded_ = true;
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
        functions_.emplace_back(function_data_t{&child_die});
        break;
      default:
        break;
      }
    } while (dwarf_siblingof(&child_die, &child_die) == 0);
  }
}

function_data_t::function_data_t(Dwarf_Die *function_die)
    : function_name{""}, source_file_name{""}, source_line{-1} {
  _load_function_name(function_die);
}

void function_data_t::_load_function_name(Dwarf_Die *function_die) {
  Dwarf_Attribute da;
  memset(&da, 0, sizeof(Dwarf_Attribute));

  dwarf_attr_integrate(function_die, DW_AT_name, &da);
  const char *fname = dwarf_formstring(&da);
  if (fname)
    strncpy(function_name, fname, sizeof(function_name));
}
