//
// Created by sergei_krotov on 7/4/19.
//

#pragma once

#ifndef YBTL_YBTL_DWARF_H
#define YBTL_YBTL_DWARF_H

#include <string>
#include <mutex>
#include <vector>
#include <unordered_set>

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

struct function_data_t {
  std::string function_name;
  std::string source_file_name;
  size_t source_line;

  explicit function_data_t(Dwarf_Die *function_die);

  bool operator==(const function_data_t &other) const;
private :
  void _load_function_name(Dwarf_Die *function_die);
  void _load_declaration_source_line(Dwarf_Die *function_die);
  void _load_source_file_name(Dwarf_Die *function_die);

  static const char *get_filename_by_cu_id(Dwarf_Die *function_die, size_t file_idx);
};

inline void hash_combine(std::size_t &) {}

template<typename T, typename... Rest>
inline void hash_combine(std::size_t &seed, const T &v, Rest... rest) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  hash_combine(seed, rest...);
}

struct function_data_t_hasher {
  std::size_t operator()(const function_data_t &fd) const {
    std::size_t result = 0;

    hash_combine(result, fd.function_name, fd.source_file_name, fd.source_line);

    return result;
  }
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
  using functions_container_t = std::unordered_set<function_data_t, function_data_t_hasher>;
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

  ExecutableDwarfData(const ExecutableDwarfData &) = delete;

  virtual ~ExecutableDwarfData() noexcept;

  bool is_initialized() const noexcept;

  bool load_dwarf_data();

  bool is_dwarf_data_loaded() const noexcept {
    return is_dwarf_data_loaded_;
  }

  functions_container_t::iterator begin() {
    return functions_.begin();
  }

  functions_container_t::iterator end() {
    return functions_.end();
  }

  functions_container_t::const_iterator cbegin() const {
    return functions_.cbegin();
  }

  functions_container_t::const_iterator cend() const {
    return functions_.cend();
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
