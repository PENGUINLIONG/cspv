#include <algorithm>
#include <set>
#include <iterator>
#include "pass/pass.hpp"
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct BlockFlatteningMutator : public Mutator {

  bool is_tail_stmt(const StmtRef& x) {
    switch (x->op) {
    case L_STMT_OP_BLOCK:
    {
      const auto& block = x->as<StmtBlock>();
      return !block.stmts.empty() && is_tail_stmt(block.stmts.back());
    }
    case L_STMT_OP_CONDITIONAL_BRANCH:
    {
      const auto& stmt = x->as<StmtConditionalBranch>();
      return is_tail_stmt(stmt.then_block) && is_tail_stmt(stmt.else_block);
    }
    case L_STMT_OP_RETURN:
    case L_STMT_OP_LOOP_MERGE:
    case L_STMT_OP_LOOP_CONTINUE:
    case L_STMT_OP_LOOP_BACK_EDGE:
      return true;
    default:
      return false;
    }
  }

  virtual StmtRef mutate_stmt_(StmtConditionalBranchRef x) override final {
    StmtBlockRef then_block = mutate_stmt(x->then_block);
    StmtBlockRef else_block = mutate_stmt(x->else_block);
    if (then_block->stmts.empty() && else_block->stmts.empty()) {
      return new StmtBlock({});
    } else {
      return new StmtConditionalBranch(x->cond, then_block, else_block);
    }
  }
  virtual StmtRef mutate_stmt_(StmtBlockRef x) override final {
    std::vector<StmtRef> stmts;
    stmts.reserve(x->stmts.size());
    for (StmtRef stmt : x->stmts) {
      stmt = mutate_stmt(stmt);
      if (stmt->is<StmtBlock>()) {
        const auto& stmts2 = stmt.as<StmtBlock>()->stmts;
        // If it's a nested block, flatten its content to remove indirection.
        for (auto stmt : stmts2) {
          stmts.emplace_back(stmt);
        }
      } else {
        stmts.emplace_back(stmt);
      }

      // If it's a tail statement, any other statement after it becomes dead
      // code and will never be reached. So it's okay to ignore them.
      if (!stmts.empty() && is_tail_stmt(stmts.back())) { break; }
    }

    return new StmtBlock(std::move(stmts));
  }

};

struct BlockFlatteningPass : public Pass {
  BlockFlatteningPass() : Pass("block-flattening") {}
  virtual void apply(NodeRef& x) override final {
    BlockFlatteningMutator v;
    x = v.mutate(x);
  }
};
static Pass* PASS = reg_pass<BlockFlatteningPass>();
