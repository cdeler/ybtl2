//
// Created by sergei_krotov on 7/4/19.
//
#include <vector>

#include "ybtl_dwarf.h"

using namespace std;
using namespace cdeler::ybtl2;

bool ExecutableDwarfData::is_initialized() const noexcept {
  return dwarf.dwarf != nullptr && elf.elf != nullptr && fd >= 0;
}

ExecutableDwarfData::~ExecutableDwarfData() noexcept {
  if (fd_opened_internally && fd >= 0) {
    close(fd);
  }
}
