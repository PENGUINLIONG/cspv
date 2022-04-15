// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.
// Expression tree visitor.
// @PENGUINLIONG
#pragma once
#include "spv/expr.hpp"

struct ExprVisitor {
  void visit_expr(const Expr& expr) {
    switch (expr.op) {
    case L_EXPR_OP_CONSTANT: visit_expr(*(const ExprConstant*)&expr); break;
    case L_EXPR_OP_LOAD: visit_expr(*(const ExprLoad*)&expr); break;
    case L_EXPR_OP_ADD: visit_expr(*(const ExprAdd*)&expr); break;
    case L_EXPR_OP_SUB: visit_expr(*(const ExprSub*)&expr); break;
    case L_EXPR_OP_LT: visit_expr(*(const ExprLt*)&expr); break;
    case L_EXPR_OP_EQ: visit_expr(*(const ExprEq*)&expr); break;
    case L_EXPR_OP_TYPE_CAST: visit_expr(*(const ExprTypeCast*)&expr); break;
    default: liong::unreachable();
    }
  }
  virtual void visit_expr(const ExprConstant&) {}
  virtual void visit_expr(const ExprLoad&) {}
  virtual void visit_expr(const ExprAdd&) {}
  virtual void visit_expr(const ExprSub&) {}
  virtual void visit_expr(const ExprLt&) {}
  virtual void visit_expr(const ExprEq&) {}
  virtual void visit_expr(const ExprTypeCast&) {}
};
