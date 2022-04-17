// Expression op registry.
// @PENGUINLIONG
#pragma once
#include "spv/type-reg.hpp"

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
  const std::shared_ptr<Type> ty;

  virtual bool is_same_as(const Type& other) const {
    liong::unimplemented();
  }

  virtual bool is_constexpr() const {
    liong::unimplemented();
  }

protected:
  inline Expr(ExprOp op, const std::shared_ptr<Type>& ty) :
    Node(L_NODE_VARIANT_EXPR), op(op), ty(ty) {}
};
