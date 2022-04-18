// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.
// Expression node registry.
// @PENGUINLIONG
#pragma once
#include "node/node.hpp"

enum ExprOp {
  L_EXPR_OP_CONSTANT,
  L_EXPR_OP_LOAD,
  L_EXPR_OP_ADD,
  L_EXPR_OP_SUB,
  L_EXPR_OP_LT,
  L_EXPR_OP_EQ,
  L_EXPR_OP_TYPE_CAST,
};

struct Expr : public Node {
  const ExprOp op;
  std::shared_ptr<Type> ty;

  template<typename T>
  const T& as() const {
    liong::assert(is<T>(), "expression op mismatched");
    return *(const T*)this;
  }
  template<typename T>
  bool is() const {
    return op == T::OP;
  }

protected:
  inline Expr(
    ExprOp op,
    const std::shared_ptr<Type>& ty
  ) : Node(L_NODE_VARIANT_EXPR),
    op(op),
    ty(ty) {}
};
