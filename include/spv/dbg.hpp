// Debug prints.
// @PENGUINLIONG
#pragma once
#include <sstream>
#include "gft/assert.hpp"
#include "spv/ast.hpp"

struct Debug {
  bool line_start = false;
  std::stringstream s;
  std::string indent;

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



inline Debug& operator<<(Debug& s, const ControlFlow& x) {
  //s << x.label.inner << ": " << std::endl;
  for (const auto& stmt : x.stmts) {
    s << *stmt << std::endl;
  }
  if (x.sel) {
    for (const auto& branch : x.sel->branches) {
      if (branch.branch_ty == L_BRANCH_TYPE_ALWAYS) {
        s << "if (true) {" << std::endl;
      } else if (branch.branch_ty == L_BRANCH_TYPE_NEVER) {
        s << "if (false) {" << std::endl;
      } else if (branch.branch_ty == L_BRANCH_TYPE_CONDITION_THEN) {
        s << "if " << *branch.cond << " {" << std::endl;
      } else if (branch.branch_ty == L_BRANCH_TYPE_CONDITION_ELSE) {
        s << "if (!" << *branch.cond << ") {" << std::endl;
      } else {
        liong::unreachable();
      }
      s.push_indent();
      s << *branch.ctrl_flow;
      s.pop_indent();
      s << "}" << std::endl;
    }
  }
  if (x.loop) {
    s << "while " << *x.loop->cond << " {" << std::endl;
    s.push_indent();
    s << *x.loop->body;
    s.pop_indent();
    s << "}" << std::endl;
  }
  if (x.next) {
    s << *x.next;
  }
  return s;
}
