//
// Created by sergei_krotov on 7/18/19.
//


#pragma once

#ifndef YBTL2_SRC_YBTL_TYPES_H
#define YBTL2_SRC_YBTL_TYPES_H

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
#include <libgen.h>

namespace cdeler::ybtl2 {

struct function_data_t {
  std::string function_name;
  std::string source_file_name;
  size_t source_line;

  explicit function_data_t(Dwarf_Die *function_die);

  explicit function_data_t(std::string function_name_,
                           std::string source_file_name_,
                           size_t source_line_)
      : function_name{std::move(function_name_)},
        source_file_name{std::move(source_file_name_)},
        source_line{source_line_} {};

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
    std::hash<std::string> hasher;

    return hasher(fd.function_name);
  }
};

}

#endif //YBTL2_SRC_YBTL_TYPES_H
