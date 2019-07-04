//
// Created by sergei_krotov on 7/4/19.
//

#pragma once

#ifndef YBTL_YBTL_DWARF_H
#define YBTL_YBTL_DWARF_H

#include <string_view>
#include <string>

#include <elfutils/libdw.h>
#include <elfutils/libdwfl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <fcntl.h>
#include <dwarf.h>
#include <libgen.h>

namespace cdeler::ybtl2 {
struct ElfHandle final {
  Elf *elf;

  explicit ElfHandle(int fd_) noexcept
      : elf{nullptr} {
    if (elf_version(EV_CURRENT) != EV_NONE && fd_ >= 0) {
      elf = elf_begin(fd_, ELF_C_READ, nullptr);
    }
  }

  ~ElfHandle() noexcept {
    if (elf != nullptr) {
      elf_end(elf);
    }
  }
};

struct DwarfHandle final {
  Dwarf *dwarf;

  explicit DwarfHandle(const ElfHandle &elf) noexcept
      : dwarf{nullptr} {
    if (elf.elf != nullptr) {
      dwarf = dwarf_begin_elf(elf.elf, DWARF_C_READ, nullptr);
    }
  }

  ~DwarfHandle() noexcept {
    if (dwarf != nullptr) {
      dwarf_end(dwarf);
    }
  }
};

class ExecutableDwarfData {
 public:
  explicit ExecutableDwarfData(int fd_) noexcept
      : fd{fd_}, fd_opened_internally{false}, elf{fd}, dwarf{elf} {}

  explicit ExecutableDwarfData(const char *executable_path)
      : ExecutableDwarfData{open(executable_path, O_RDONLY)} {
    fd_opened_internally = true;
  }

  explicit ExecutableDwarfData(const std::string_view &sw)
      : ExecutableDwarfData{sw.data()} {}

  explicit ExecutableDwarfData(const std::string &s)
      : ExecutableDwarfData{s.data()} {}

  ExecutableDwarfData(const ExecutableDwarfData &) = delete;

  virtual ~ExecutableDwarfData() noexcept;

  bool is_initialized() const noexcept;
 private:
  int fd;
  bool fd_opened_internally;

  ElfHandle elf;
  DwarfHandle dwarf;
};
}
#endif //YBTL_YBTL_DWARF_H
