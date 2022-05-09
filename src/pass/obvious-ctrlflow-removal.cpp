#include <algorithm>z
#include <set>
#include <iterator>
#include "pass/pass.hpp"
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct ObviousCtrlflowRemovalMutator : public Mutator {

  StmtRef remove_tail_continue(StmtRef x) {

    if (x->is<StmtConditionalBranch>()) {
      StmtConditionalBranchRef branch = x;
      remove_tail_continue(branch->then_block);
      remove_tail_continue(branch->else_block);

    } else if (x->is<StmtBlock>()) {
      StmtBlockRef block = x;
      assert(!block->stmts.empty());

      StmtRef& tail_stmt = block->stmts.back();
      if (tail_stmt->is<StmtLoopContinue>()) {
        block->stmts.pop_back();
      } else if (tail_stmt->is<StmtBlock>()) {
        remove_tail_continue(tail_stmt);
      } else if (tail_stmt->is<StmtConditionalBranch>()) {
        remove_tail_continue(tail_stmt);
      } else if (tail_stmt->is<StmtLoopMerge>()) {
        // Ignore breaks.
      } else {
        unreachable();
      }
    }

    return x;
  }
  virtual StmtRef mutate_stmt_(StmtLoopRef x) override final {
    {
      StmtBlockRef body_block = x->body_block;
      assert(!body_block->stmts.empty());

      StmtRef& tail_stmt = body_block->stmts.back();
      if (tail_stmt->is<StmtLoopContinue>()) {
        body_block->stmts.pop_back();
      } else if (tail_stmt->is<StmtConditionalBranch>()) {
        tail_stmt = remove_tail_continue(tail_stmt);
      } else {
        unreachable();
      }
    }

    {
      StmtBlockRef continue_block = x->continue_block;
      assert(!continue_block->stmts.empty());

      StmtRef& tail_stmt = continue_block->stmts.back();
      if (tail_stmt->is<StmtLoopBackEdge>()) {
        continue_block->stmts.pop_back();
      } else {
        unreachable();
      }
    }

    return x;
  }

};

struct ObviousCtrlflowRemovalPass : public Pass {
  ObviousCtrlflowRemovalPass() : Pass("obvious-ctrlflow-removal") {}
  virtual void apply(NodeRef& x) override final {
    ObviousCtrlflowRemovalMutator v;
    x = v.mutate(x);
  }
};
static Pass* PASS = reg_pass<ObviousCtrlflowRemovalPass>();
