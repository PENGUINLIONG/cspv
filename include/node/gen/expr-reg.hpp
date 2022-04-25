// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.
// Expr node registry.
// @PENGUINLIONG
#pragma once
#include "node/node.hpp"

enum ExprOp {
  L_EXPR_OP_PATTERN_CAPTURE,
  L_EXPR_OP_PATTERN_BINARY_OP,
  L_EXPR_OP_BOOL_IMM,
  L_EXPR_OP_INT_IMM,
  L_EXPR_OP_FLOAT_IMM,
  L_EXPR_OP_LOAD,
  L_EXPR_OP_ADD,
  L_EXPR_OP_SUB,
  L_EXPR_OP_MUL,
  L_EXPR_OP_DIV,
  L_EXPR_OP_MOD,
  L_EXPR_OP_LT,
  L_EXPR_OP_EQ,
  L_EXPR_OP_NOT,
  L_EXPR_OP_TYPE_CAST,
  L_EXPR_OP_SELECT,
};

struct Expr : public Node {
  const ExprOp op;
  TypeRef ty;

  template<typename T>
  const T& as() const {
    liong::assert(is<T>(), "expr op mismatched");
    return *(const T*)this;
  }
  template<typename T>
  bool is() const {
    return op == T::OP;
  }

protected:
  inline Expr(
    ExprOp op,
    const TypeRef& ty
  ) : Node(L_NODE_VARIANT_EXPR), op(op), ty(ty)
  {
    liong::assert(ty != nullptr);
  }
};
