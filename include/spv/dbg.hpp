// Debug prints.
// @PENGUINLIONG
#pragma once
#include <ostream>
#include "gft/assert.hpp"
#include "spv/ast.hpp"

inline std::ostream& operator<<(std::ostream& s, const ControlFlow& x) {
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
      s << *branch.ctrl_flow;
      s << "}" << std::endl;
    }
  }
  if (x.loop) {
    s << "while " << *x.loop->cond << " {" << std::endl;
    s << *x.loop->body;
    s << "}" << std::endl;
  }
  if (x.next) {
    s << *x.next;
  }
  return s;
}
