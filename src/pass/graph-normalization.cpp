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

  static bool is_tail_stmt(const StmtRef& x) {
    switch (x->op) {
    case L_STMT_OP_BLOCK:
      return is_tail_stmt(x->as<StmtBlock>().stmts.back());
    case L_STMT_OP_CONDITIONAL_BRANCH:
    {
      const auto& stmt = x->as<StmtConditionalBranch>();
      return is_tail_stmt(stmt.then_block) && is_tail_stmt(stmt.else_block);
    }
    case L_STMT_OP_RETURN:
    case L_STMT_OP_IF_THEN_ELSE_MERGE:
    case L_STMT_OP_LOOP_MERGE:
    case L_STMT_OP_LOOP_CONTINUE:
    case L_STMT_OP_LOOP_BACK_EDGE:
      return true;
    default:
      return false;
    }
  }
  virtual StmtRef mutate_stmt_(StmtBlockRef x) override final {
    std::vector<StmtRef> stmts;
    stmts.reserve(x->stmts.size());
    for (StmtRef& stmt : x->stmts) {
      StmtRef stmt2 = mutate_stmt(stmt);

      if (stmt2->is<StmtBlock>()) {
        const auto& stmts2 = stmt2.as<StmtBlock>()->stmts;
        // If it's a nested block, flatten its content to remove indirection.
        for (auto stmt : stmts2) {
          stmts.emplace_back(stmt);
        }
      } else if (stmt2->is<StmtNop>()) {
        // Ignore nops.
        continue;
      } else {
        stmts.emplace_back(stmt2);
      }

      // If it's a tail statement, any other statement after it becomes dead
      // code and will never be reached. So it's okay to ignore them.
      if (is_tail_stmt(stmts.back())) { break; }
    }

    switch (stmts.size()) {
    case 0: return new StmtNop();
    case 1: return std::move(stmts[0]);
    default: return new StmtBlock(std::move(stmts));
    }
  }

  virtual StmtRef mutate_stmt_(StmtConditionalBranchRef x) override final {
    x->cond = mutate_expr(x->cond);
    x->then_block = mutate_stmt(x->then_block);
    x->else_block = mutate_stmt(x->else_block);

    if (x->then_block->is<StmtIfThenElseMerge>() && x->else_block->is<StmtIfThenElseMerge>()) {
      return new StmtNop;
    } else {
      return x;
    }
  }

  virtual StmtRef mutate_stmt_(StmtIfThenElseRef x) override final {
    x->body_block = mutate_stmt(x->body_block);

    switch (x->body_block->op) {
    case L_STMT_OP_CONDITIONAL_BRANCH:
      return x;
    default: 
      // If-then-else statement should have a conditional branch as its child.
      // But if the child is not a conditional branch, it becomes a linear
      // statement stream which never forms an if-then-else-construct.
      return x->body_block;
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
