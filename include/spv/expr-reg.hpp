// Expression op registry.
// @PENGUINLIONG
#pragma once
#include "spv/type-reg.hpp"

enum ExprOp {
  L_EXPR_OP_CONSTANT,

  L_EXPR_OP_LOAD,

  L_EXPR_OP_NEG,
  L_EXPR_OP_ADD,
  L_EXPR_OP_SUB,
  L_EXPR_OP_MUL,
  L_EXPR_OP_DIV,
  L_EXPR_OP_MOD,

  L_EXPR_OP_LT,
  L_EXPR_OP_EQ,

  L_EXPR_OP_AND,
  L_EXPR_OP_OR,
  L_EXPR_OP_NOT,
  L_EXPR_OP_XOR,

  L_EXPR_OP_TYPE_CAST,
};
struct Expr {
  const ExprOp op;
  const std::shared_ptr<Type> ty;

  virtual bool is_same_as(const Type& other) const {
    liong::unimplemented();
  }

  virtual void dbg_print(Debug& s) const { s << "expr?"; }

protected:
  inline Expr(ExprOp op, const std::shared_ptr<Type>& ty) : op(op), ty(ty) {}
};

inline Debug& operator<<(Debug& s, const Expr& x) {
  x.dbg_print(s);
  return s;
}
