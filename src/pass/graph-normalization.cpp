// For any binary expression, if the expression has a constant operand and a
// non-constant operand, ensure the constant operand is always on the right-
// hand-side (`b`).
//
// @PENGUINLIONG
#include "pass/pass.hpp"
#include "visitor/util.hpp"

struct GraphNormalizationMutator : Mutator {
  virtual ExprRef mutate_expr_(ExprAddRef x) override final {
    if (x->a->is<ExprConstant>() && !x->b->is<ExprConstant>()) {
      x->a = std::exchange(x->b, std::move(x->a));
      return x.as<Expr>();
    }
    return Mutator::mutate_expr_(x);
  }
  virtual ExprRef mutate_expr_(ExprEqRef x) override final {
    if (x->a->is<ExprConstant>() && !x->b->is<ExprConstant>()) {
      x->a = std::exchange(x->b, std::move(x->a));
      return x.as<Expr>();
    }
    return Mutator::mutate_expr_(x);
  }
  virtual ExprRef mutate_expr_(ExprLtRef x) override final {
    if (x->a->is<ExprConstant>() && !x->b->is<ExprConstant>()) {
      x->a = std::exchange(x->b, std::move(x->a));
      return x.as<Expr>();
    }
    return Mutator::mutate_expr_(x);
  }

  virtual StmtRef mutate_stmt_(StmtBlockRef x) override final {
    x = Mutator::mutate_stmt_(x);
    return flatten_block(x);
  }

};


struct GraphNormalizationPass : public Pass {
  GraphNormalizationPass() : Pass("graph-normalization") {}
  virtual void apply(NodeRef<Node>& x) override final {
    GraphNormalizationMutator v;
    x = v.mutate(x);
  }
};
static Pass* PASS = reg_pass<GraphNormalizationPass>();
