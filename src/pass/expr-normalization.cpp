// For any binary expression, if the expression has a constant operand and a
// non-constant operand, ensure the constant operand is always on the right-
// hand-side (`b`).
//
// @PENGUINLIONG
#include "pass/pass.hpp"

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
    std::vector<StmtRef> stmts;
    stmts.reserve(x->stmts.size());
    for (StmtRef& stmt : x->stmts) {
      StmtRef stmt2 = mutate_stmt(stmt);

      if (stmt2->is<StmtBlock>()) {
        // If it's a nested block, flatten its content to remove indirection.
        for (auto stmt : stmt2.as<StmtBlock>()->stmts) {
          stmts.emplace_back(stmt);
        }
      } else if (!stmt2->is<StmtNop>()) {
        // Ignore any nops.
        stmts.emplace_back(stmt2);
      }
    }

    switch (stmts.size()) {
    case 0: return new StmtNop();
    case 1: return std::move(stmts[0]);
    default: return new StmtBlock(std::move(stmts));
    }

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
