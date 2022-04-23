#include <set>
#include "gft/log.hpp"
#include "pass/pass.hpp"
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct CtrlflowLinearizationMutator : public Mutator {

  std::shared_ptr<uint8_t> outer_loop_handle;

  const StmtRef& get_tail_stmt(const StmtRef& stmt) {
    if (stmt->is<StmtBlock>()) {
      const auto& block = stmt->as<StmtBlock>();
      assert(!block.stmts.empty());
      return get_tail_stmt(block.stmts.back());
    } else {
      return stmt;
    }
  }
  virtual StmtRef mutate_stmt_(StmtConditionalBranchRef x) override final {
    ExprRef cond = mutate_expr(x->cond);
    StmtRef then_block = mutate_stmt(x->then_block);
    StmtRef else_block = mutate_stmt(x->else_block);

    const StmtRef& then_tail = get_tail_stmt(then_block);
    const StmtRef& else_tail = get_tail_stmt(else_block);

    if (!then_tail->is<StmtLoopMerge>() && else_tail->is<StmtLoopMerge>()) {
      auto out = new StmtBlock({
        new StmtConditionalBranch(new ExprNot(cond->ty, cond), else_block, new StmtNop)
        });
      if (then_block->is<StmtBlock>()) {
        for (auto& stmt : then_block->as<StmtBlock>().stmts) {
          out->stmts.emplace_back(stmt);
        }
      } else {
        out->stmts.emplace_back(then_block);
      }
      return out;
    }

    return new StmtConditionalBranch(cond, then_block, else_block);
  }

};

struct CtrlflowLinearizationPass : public Pass {
  CtrlflowLinearizationPass() : Pass("ctrlflow-linearization") {}
  virtual void apply(NodeRef<Node>& x) override final {
    CtrlflowLinearizationMutator v;
    x = v.mutate(x);
  }
};
static Pass* PASS = reg_pass<CtrlflowLinearizationPass>();
