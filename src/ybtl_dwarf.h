//
// Created by sergei_krotov on 7/4/19.
//

#pragma once

#ifndef YBTL_YBTL_DWARF_H
#define YBTL_YBTL_DWARF_H

#include <string>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <iostream>

#include <elfutils/libdw.h>
#include <elfutils/libdwfl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <fcntl.h>
#include <dwarf.h>
#include "ybtl_types.h"

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
  using functions_container_t = std::unordered_map<std::size_t, function_data_t>;
public:
  explicit ExecutableDwarfData(int fd_) noexcept
      : fd_{fd_},
        fd_opened_internally_{false},
        elf_{fd_},
        dwarf_{elf_},
        is_dwarf_data_loaded_{false} {}

  explicit ExecutableDwarfData(const char *executable_path)
      : ExecutableDwarfData{open(executable_path, O_RDONLY)} {
    fd_opened_internally_ = true;
  }

  explicit ExecutableDwarfData(const std::string &s)
      : ExecutableDwarfData{s.data()} {}

  explicit ExecutableDwarfData()
      : ExecutableDwarfData{"/proc/self/exe"} {}

  ExecutableDwarfData(const ExecutableDwarfData &) = delete;

  virtual ~ExecutableDwarfData() noexcept;

  bool is_initialized() const noexcept;

  bool load_dwarf_data();

  bool is_dwarf_data_loaded() const noexcept {
    return is_dwarf_data_loaded_;
  }

  functions_container_t::const_iterator find(const std::size_t function_start_address) const {
    return functions_.find(function_start_address);
  }

  functions_container_t::const_iterator cbegin() const {
    return functions_.cbegin();
  }

  functions_container_t::const_iterator cend() const {
    return functions_.cend();
  }

  functions_container_t::iterator begin() {
    return functions_.begin();
  }

  functions_container_t::iterator end() {
    return functions_.end();
  }
private:
  void _read_dwarf_data();

  int fd_;
  bool fd_opened_internally_;

  ElfHandle elf_;
  DwarfHandle dwarf_;

  volatile bool is_dwarf_data_loaded_;
  std::mutex data_load_mutex_;
  functions_container_t functions_{};
};

}

#endif //YBTL_YBTL_DWARF_H
