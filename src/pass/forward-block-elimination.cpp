#include "gft/log.hpp"
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

// Unwrap any sole member in block statements.
struct ForwardBlockEliminationMutator : public StmtMutator {
  virtual StmtRef mutate_stmt_(StmtBlockRef& x) override final {
    StmtMutator::mutate_stmt_(x);
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
