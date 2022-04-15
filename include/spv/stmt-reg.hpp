// Statements op registry.
// @PENGUINLIONG
#pragma once
#include "spv/mem-reg.hpp"
#include "spv/expr-reg.hpp"

enum StmtOp {
  L_STMT_OP_STORE,
};
struct Stmt {
  const StmtOp op;

  virtual void visit(struct StmtVisitor*) const {}

  virtual bool is_same_as(const Type& other) const {
    liong::unimplemented();
  }

  virtual void dbg_print(Debug& s) const { s << "stmt?"; }

protected:
  inline Stmt(StmtOp op) : op(op) {}
};

inline Debug& operator<<(Debug& s, const Stmt& x) {
  x.dbg_print(s);
  return s;
}
