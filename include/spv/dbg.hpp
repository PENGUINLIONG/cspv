// Debug prints.
// @PENGUINLIONG
#pragma once
#include <sstream>
#include "gft/assert.hpp"

struct Debug {
  bool line_start = false;
  std::stringstream s;
  std::string indent;

  std::map<const void*, std::string> var_handle2name_map;

  inline const std::string& get_var_name_by_handle(const void* handle) {
    auto it = var_handle2name_map.find(handle);
    if (it == var_handle2name_map.end()) {
      std::string name = "_";
      name += std::to_string(var_handle2name_map.size());
      var_handle2name_map.emplace(handle, std::move(name));
    }
    return var_handle2name_map.at(handle);
  }

  inline void push_indent() { indent += "  "; }
  inline void pop_indent() { indent.resize(indent.size() - 2); }
  
  template<typename T>
  inline Debug& operator<<(const T& x) {
    if (line_start) {
      s << indent;
      line_start = false;
    }
    s << x;
    return *this;
  }
  inline Debug& operator<<(std::ostream& (*f)(std::ostream&)) {
    if (f == std::endl<std::stringstream::char_type, std::stringstream::traits_type>) {
      s << indent << "\n";
      line_start = true;
      return *this;
    }
    f(s);
    return *this;
  }

  inline std::string str() const { return s.str(); }
};
