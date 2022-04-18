#include "gft/log.hpp"
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

struct ForwardBlockEliminationMutator : public StmtMutator {
  virtual StmtRef mutate_stmt_(StmtBlockRef& x) override final {
    for (size_t i = 0; i < x->stmts.size(); ++i) {
      auto it = x->stmts.begin() + i;
      *it = StmtMutator::mutate_stmt(*it);
      if ((*it)->is<StmtBlock>()) {
        auto stmts = std::move((*it)->as<StmtBlock>().stmts);
        x->stmts.insert(x->stmts.erase(it), stmts.begin(), stmts.end());
      }
    }

    if (x->stmts.size() == 1) {
      return std::move(x->stmts[0]);
    }
    return x;
  }
};
StmtRef eliminate_forward_blocks(StmtRef& x) {
  ForwardBlockEliminationMutator mutator;
  return mutator.mutate_stmt(x);
}
