#include <set>
#include "gft/log.hpp"
#include "pass/pass.hpp"
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct CtrlflowLinearizationMutator : public Mutator {

  std::shared_ptr<uint8_t> outer_loop_handle;

  StmtRef& get_tail_stmt(StmtRef& stmt) {
    if (stmt->is<StmtBlock>()) {
      auto& block = stmt->as<StmtBlock>();
      assert(!block.stmts.empty());
      return get_tail_stmt((StmtRef&)block.stmts.back());
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
  virtual StmtRef mutate_stmt_(StmtLoopRef x) override final {
    x->body_block = mutate_stmt(x->body_block);
    StmtRef& body_tail = get_tail_stmt(x->body_block);
    if (body_tail->is<StmtLoopContinue>()) {
      body_tail = StmtRef(new StmtNop);
    }
    x->body_block = flatten_block(x->body_block);

    x->continue_block = mutate_stmt(x->continue_block);
    StmtRef& continue_tail = get_tail_stmt(x->continue_block);
    if (continue_tail->is<StmtLoopBackEdge>()) {
      continue_tail = StmtRef(new StmtNop);
    }
    x->continue_block = flatten_block(x->continue_block);

    return x;
  }
  virtual StmtRef mutate_stmt_(StmtBlockRef x) override final {
    x = Mutator::mutate_stmt_(x);
    return flatten_block(x);
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
