#include <algorithm>
#include <set>
#include <iterator>
#include "pass/pass.hpp"
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct LoopComprehensionMutator : public Mutator {

  virtual StmtRef mutate_stmt_(StmtLoopRef x) override final {
    StmtRef out;
    x = Mutator::mutate_stmt_(x);

    ExprRef cond;
    StmtBlockRef body_block = x->body_block;
    if (!body_block->stmts.empty() && body_block->stmts.back()->is<StmtConditionalBranch>()) {
      StmtConditionalBranchRef branch = body_block->stmts.back();
      StmtBlockRef then_block = branch->then_block;
      StmtBlockRef else_block = branch->else_block;
      if (else_block->stmts.size() == 1 && else_block->stmts.back()->is<StmtLoopMerge>()) {
        std::vector<StmtRef> body_stmts(
          body_block->stmts.begin(),
          body_block->stmts.end() - 1);
        for (const StmtRef& stmt : then_block->stmts) {
          body_stmts.emplace_back(stmt);
        }
        out = new StmtConditionalLoop(
          branch->cond,
          new StmtBlock(body_stmts),
          x->continue_block,
          x->handle);
      } else {
        out = x;
      }
    } else {
      out = x;
    }

    return out;
  }

};

struct LoopComprehensionPass : public Pass {
  LoopComprehensionPass() : Pass("loop-comprehension") {}
  virtual void apply(NodeRef& x) override final {
    LoopComprehensionMutator v;
    x = v.mutate(x);
  }
};
static Pass* PASS = reg_pass<LoopComprehensionPass>();
