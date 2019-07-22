//
// Created by sergei_krotov on 7/18/19.
//

#include "ybtl_types.h"

using namespace cdeler::ybtl2;

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
static const constexpr size_t STACK_WALKER_IDENTEFER_NAME_MAX_LENGTH __attribute__((unused)) = 64;

function_data_t::function_data_t(Dwarf_Die *function_die)
    : function_name{}, function_address{0}, source_file_name{}, source_line{0} {
  _load_function_name(function_die);
  _load_function_address(function_die);
  _load_source_file_name(function_die);
  _load_declaration_source_line(function_die);
}
void function_data_t::_load_function_name(Dwarf_Die *function_die) {
  Dwarf_Attribute da;
  memset(&da, 0, sizeof(Dwarf_Attribute));

  dwarf_attr_integrate(function_die, DW_AT_name, &da);
  const char *fname = dwarf_formstring(&da);
  if (fname)
    function_name = fname;

  if (function_name == "b0e7d6d2_1535_4fe9_9a1c_60f538b7e564") {
    int i = 5;
  }
}

void function_data_t::_load_declaration_source_line(Dwarf_Die *function_die) {
  Dwarf_Attribute da;
  memset(&da, 0, sizeof(Dwarf_Attribute));

  dwarf_attr_integrate(function_die, DW_AT_decl_line, &da);
  dwarf_formudata(&da, &source_line);
}

void function_data_t::_load_function_address(Dwarf_Die *function_die) {
  Dwarf_Attribute da;
  memset(&da, 0, sizeof(Dwarf_Attribute));
  Dwarf_Addr value;
  dwarf_attr_integrate(function_die, DW_AT_low_pc, &da);

  if (dwarf_formaddr(&da, &value) == 0)
    function_address = value;
}

void function_data_t::_load_source_file_name(Dwarf_Die *function_die) {
  Dwarf_Attribute da;
  std::size_t file_idx;

  memset(&da, 0, sizeof(Dwarf_Attribute));

  dwarf_attr_integrate(function_die, DW_AT_decl_file, &da);
  dwarf_formudata(&da, &file_idx);

  char *file_name = const_cast<char *>( get_filename_by_cu_id(function_die, file_idx));

  if (file_name)
    source_file_name = basename(file_name);
}

const char *function_data_t::get_filename_by_cu_id(Dwarf_Die *function_die, std::size_t file_idx) {
  Dwarf_Die cu_die;
  Dwarf_Files *files;

  if (!dwarf_diecu(function_die, &cu_die, nullptr, nullptr) ||
      dwarf_getsrcfiles(&cu_die, &files, nullptr) != 0)
    return nullptr;

  return dwarf_filesrc(files, file_idx, nullptr, nullptr);
}

bool function_data_t::operator==(const function_data_t &other) const {
  return (function_address == other.function_address)
      && (source_line == other.source_line)
      && (function_name == other.function_name)
      && (source_file_name == other.source_file_name);
}