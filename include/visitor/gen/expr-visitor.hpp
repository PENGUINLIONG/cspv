// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.
// Expression tree visitor.
// @PENGUINLIONG
#pragma once
#include "spv/expr.hpp"

struct ExprVisitor {
  virtual void visit_expr(const Expr& expr) {
    switch (expr.op) {
    case L_EXPR_OP_CONSTANT: visit_expr_(*(const ExprConstant*)&expr); break;
    case L_EXPR_OP_LOAD: visit_expr_(*(const ExprLoad*)&expr); break;
    case L_EXPR_OP_ADD: visit_expr_(*(const ExprAdd*)&expr); break;
    case L_EXPR_OP_SUB: visit_expr_(*(const ExprSub*)&expr); break;
    case L_EXPR_OP_LT: visit_expr_(*(const ExprLt*)&expr); break;
    case L_EXPR_OP_EQ: visit_expr_(*(const ExprEq*)&expr); break;
    case L_EXPR_OP_TYPE_CAST: visit_expr_(*(const ExprTypeCast*)&expr); break;
    default: liong::unreachable();
    }
  }
  virtual void visit_expr_(const ExprConstant&);
  virtual void visit_expr_(const ExprLoad&);
  virtual void visit_expr_(const ExprAdd&);
  virtual void visit_expr_(const ExprSub&);
  virtual void visit_expr_(const ExprLt&);
  virtual void visit_expr_(const ExprEq&);
  virtual void visit_expr_(const ExprTypeCast&);
};

template<typename TExpr>
struct ExprFunctorVisitor : public ExprVisitor {
  std::function<void(const TExpr&)> f;
  ExprFunctorVisitor(std::function<void(const TExpr&)>&& f) :
    f(std::forward<std::function<void(const TExpr&)>>(f)) {}

  virtual void visit_expr_(const TExpr& expr) override final {
    f(expr);
  }
};
template<typename TExpr>
void visit_expr_functor(
  std::function<void(const TExpr&)>&& f,
  const Expr& x
) {
  ExprFunctorVisitor<TExpr> visitor(
    std::forward<std::function<void(const TExpr&)>>(f));
  visitor.visit_expr(x);
}
