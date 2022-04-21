#include "pass/pass.hpp"

struct ExprNormalization : Mutator {
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

void expr_normalization(StmtRef& x) {
  ExprNormalization v;
  x = v.mutate(x);
}
