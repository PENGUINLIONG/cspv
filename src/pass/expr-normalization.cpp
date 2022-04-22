// For any binary expression, if the expression has a constant operand and a
// non-constant operand, ensure the constant operand is always on the right-
// hand-side (`b`).
//
// @PENGUINLIONG
#include "pass/pass.hpp"

struct ExprNormalizationMutator : Mutator {
  virtual ExprRef mutate_expr_(ExprAddRef& x) override final {
    if (x->a->is<ExprConstant>() && !x->b->is<ExprConstant>()) {
      x->a = std::exchange(x->b, std::move(x->a));
      return x.as<Expr>();
    }
    return Mutator::mutate_expr_(x);
  }
  virtual ExprRef mutate_expr_(ExprEqRef& x) override final {
    if (x->a->is<ExprConstant>() && !x->b->is<ExprConstant>()) {
      x->a = std::exchange(x->b, std::move(x->a));
      return x.as<Expr>();
    }
    return Mutator::mutate_expr_(x);
  }
  virtual ExprRef mutate_expr_(ExprLtRef& x) override final {
    if (x->a->is<ExprConstant>() && !x->b->is<ExprConstant>()) {
      x->a = std::exchange(x->b, std::move(x->a));
      return x.as<Expr>();
    }
    return Mutator::mutate_expr_(x);
  }
};

struct ExprNormalizationPass : public Pass {
  ExprNormalizationPass() : Pass("expr-normalization") {}
  virtual void apply(NodeRef<Node>& x) override final {
    ExprNormalizationMutator v;
    x = v.mutate(x);
  }
};
static Pass* PASS = reg_pass<ExprNormalizationPass>();
