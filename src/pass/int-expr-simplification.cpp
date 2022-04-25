// Simplify integer expressions.
// Depends on `graph-normalization` to ensure the constants are always on the
// right (`b`).
// @PENGUINLIONG
#include "pass/pass.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct IntExprSimplificationMutator : public Mutator {
  virtual ExprRef mutate_expr_(ExprAddRef x) override final {
    if (!x->ty->is<TypeInt>()) { return x; }
    ExprRef a = mutate_expr(x->a);
    ExprRef b = mutate_expr(x->b);

    if (b->is<ExprIntImm>()) {
      int64_t b_lit = ExprIntImmRef(b)->lit;
      if (a->is<ExprIntImm>()) {
        return new ExprIntImm(x->ty, ExprIntImmRef(a)->lit + b_lit);
      } else if (a->is<ExprAdd>()) {
        ExprAddRef a2 = a;
        if (a2->b->is<ExprIntImm>()) {
          int64_t ab_lit = ExprIntImmRef(a2->b)->lit;
          return new ExprAdd(x->ty, a2->a, new ExprIntImm(x->ty, b_lit + ab_lit));
        }
      }
    }

    x->a = std::move(a);
    x->b = std::move(b);
    return x;
  }
};

struct IntExprSimplificationPass : public Pass {
  IntExprSimplificationPass() : Pass("int-expr-simplification") {}
  virtual void apply(NodeRef& x) override final {
    IntExprSimplificationMutator v;
    x = v.mutate(x);
  }
};
static Pass* PASS = reg_pass<IntExprSimplificationPass>();
