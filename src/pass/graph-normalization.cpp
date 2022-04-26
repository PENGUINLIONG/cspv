// For any binary expression, if the expression has a constant operand and a
// non-constant operand, ensure the constant operand is always on the right-
// hand-side (`b`).
//
// @PENGUINLIONG
#include "pass/pass.hpp"
#include "visitor/util.hpp"

struct GraphNormalizationMutator : Mutator {
  void prioritize_binary_op_var(ExprRef& a, ExprRef& b) {
    auto a2 = mutate_expr(a);
    auto b2 = mutate_expr(b);
    if (is_expr_constant(a2->op) && !(is_expr_constant(b2->op))) {
      a2 = std::exchange(b2, std::move(a2));
    }
    a = std::move(a2);
    b = std::move(b2);
  }
  virtual ExprRef mutate_expr_(ExprAddRef x) override final {
    prioritize_binary_op_var(x->a, x->b);
    return x;
  }
  virtual ExprRef mutate_expr_(ExprMulRef x) override final {
    prioritize_binary_op_var(x->a, x->b);
    return x;
  }
  virtual ExprRef mutate_expr_(ExprEqRef x) override final {
    prioritize_binary_op_var(x->a, x->b);
    return x;
  }
  virtual ExprRef mutate_expr_(ExprLtRef x) override final {
    prioritize_binary_op_var(x->a, x->b);
    return x;
  }

  virtual StmtRef mutate_stmt_(StmtBlockRef x) override final {
    x = Mutator::mutate_stmt_(x);
    return flatten_block(x);
  }

};


struct GraphNormalizationPass : public Pass {
  GraphNormalizationPass() : Pass("graph-normalization") {}
  virtual void apply(NodeRef& x) override final {
    GraphNormalizationMutator v;
    x = v.mutate(x);
  }
};
static Pass* PASS = reg_pass<GraphNormalizationPass>();
