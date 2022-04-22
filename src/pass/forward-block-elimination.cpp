// Unwrap any sole member in block statements.
//
// ```
// StmtLoop
// +-StmtBlock
// | +-StmtBranchConditional
// |   +-StmtBlock
// |   | +-StmtLoopContinue
// |   +-StmtBlock
// |     +-StmtLoopMerge
// +-StmtBlock
//   +-StmtLoopMerge
// ```
//
// becomes:
//
// ```
// StmtLoop
// +-StmtBranchConditional
// | +-StmtLoopContinue
// | +-StmtLoopMerge
// +-StmtLoopMerge
// ```
//
// @PENGUINLIONG
#include "pass/pass.hpp"

struct ForwardBlockEliminationMutator : public Mutator {
  virtual StmtRef mutate_stmt_(StmtBlockRef& x) override final {
    for (size_t i = 0; i < x->stmts.size(); ++i) {
      auto it = x->stmts.begin() + i;
      *it = Mutator::mutate(*it);
      if ((*it)->is<StmtBlock>()) {
        auto stmts = std::move((*it)->as<StmtBlock>().stmts);
        x->stmts.insert(x->stmts.erase(it), stmts.begin(), stmts.end());
      }
    }

    if (x->stmts.size() == 1) {
      return std::move(x->stmts[0]);
    }
    return x.as<Stmt>();
  }
};

struct ForwardBlockEliminationPass : public Pass {
  ForwardBlockEliminationPass() : Pass("forward-block-elimination") {}
  virtual void apply(NodeRef<Node>& x) override final {
    ForwardBlockEliminationMutator mutator;
    x = mutator.mutate(x);
  }
};
static Pass* PASS = reg_pass<ForwardBlockEliminationPass>();
