//
// Created by sergei_krotov on 7/4/19.
//

#pragma once

#ifndef YBTL_YBTL_DWARF_H
#define YBTL_YBTL_DWARF_H

#include <string_view>
#include <string>
#include <mutex>
#include <vector>

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
/*
 * ISO/IEC 9899:2011, section §5.2.4.1 Translation limits said
        — 63 significant initial characters in an internal identifier or a macro name
        (each universal character name or
        extended source character is considered a single character)
        — 31 significant initial characters in an external identifier
        (each universal character name specifying a short identifier
        of 0000FFFF or less is considered 6 characters,
        each universal character name specifying
        a short identifier of 00010000 or more is considered 10 characters,
        and each extended source character is considered the same number
        of characters as the corresponding universal character name, if any)
 */
static const constexpr size_t STACK_WALKER_IDENTEFER_NAME_MAX_LENGTH = 64;

static constexpr const size_t DWARF_SOURCE_FILE_NAME_MAX_LENGTH = 64;
struct function_data_t {
  char function_name[STACK_WALKER_IDENTEFER_NAME_MAX_LENGTH];
  char source_file_name[DWARF_SOURCE_FILE_NAME_MAX_LENGTH];
  ssize_t source_line;

  explicit function_data_t(Dwarf_Die *function_die);

private :
  void _load_function_name(Dwarf_Die *function_die);
};

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
      : fd_{fd_}, fd_opened_internally_{false}, elf_{fd_}, dwarf_{elf_}, is_dwarf_data_loaded_{false} {}

  explicit ExecutableDwarfData(const char *executable_path)
      : ExecutableDwarfData{open(executable_path, O_RDONLY)} {
    fd_opened_internally_ = true;
  }

  explicit ExecutableDwarfData(const std::string_view &sw)
      : ExecutableDwarfData{sw.data()} {}

  explicit ExecutableDwarfData(const std::string &s)
      : ExecutableDwarfData{s.data()} {}

  ExecutableDwarfData(const ExecutableDwarfData &) = delete;

  virtual ~ExecutableDwarfData() noexcept;

  bool is_initialized() const noexcept;

  bool load_dwarf_data();

  bool is_dwarf_data_loaded() const noexcept {
    return is_dwarf_data_loaded_;
  }

  const std::vector<function_data_t> &get_function_data() const noexcept {
    return functions_;
  }
private:
  void _read_dwarf_data();

  int fd_;
  bool fd_opened_internally_;

  ElfHandle elf_;
  DwarfHandle dwarf_;

  volatile bool is_dwarf_data_loaded_;
  std::mutex data_load_mutex_;
  std::vector<function_data_t> functions_;
};

}
#endif //YBTL_YBTL_DWARF_H
