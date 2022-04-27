#include <set>
#include "pass/pass.hpp"
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct CtrlflowStmt2ExprMutator : public Mutator {
  std::map<std::shared_ptr<uint8_t>, ExprRef> func_vars {};

  virtual ExprRef mutate_expr_(ExprLoadRef x) override final {
    if (x->src_ptr->is<MemoryFunctionVariable>()) {
      MemoryFunctionVariableRef src_ptr = x->src_ptr;
      return func_vars.at(src_ptr->handle);
    } else {
      return x;
    }
  }

  virtual StmtRef mutate_stmt_(StmtBlockRef x) override final {
    std::vector<StmtRef> out_stmts;
    for (StmtRef stmt : x->stmts) {
      if (stmt->is<StmtStore>()) {
        StmtStoreRef stmt2 = stmt;
        if (stmt2->dst_ptr->is<MemoryFunctionVariable>()) {
          MemoryFunctionVariableRef dst_ptr = stmt2->dst_ptr;
          func_vars[dst_ptr->handle] = mutate_expr(stmt2->value);
        } else {
          out_stmts.emplace_back(mutate_stmt(stmt));
        }
      } else if (stmt->is<StmtConditionalBranch>()) {
        StmtConditionalBranchRef stmt2 = stmt;
        ExprRef cond = mutate_expr(stmt2->cond);

        auto func_vars2 = func_vars;
        StmtRef then_block = mutate_stmt(stmt2->then_block);
        auto func_vars_then = std::move(func_vars);

        func_vars = std::move(func_vars2);
        StmtRef else_block = mutate_stmt(stmt2->else_block);
        auto func_vars_else = std::move(func_vars);

        // Mapping from handle to function variable. A variable must exist in
        // both branch to be picked into the outer scope.
        std::set<std::shared_ptr<uint8_t>> common_handles;
        for (const auto& pair: func_vars_then) {
          common_handles.insert(pair.first);
        }
        for (const auto& pair: func_vars_else) {
          common_handles.insert(pair.first);
        }

        for (const auto& handle : common_handles) {
          const auto& then_value = func_vars_then.at(handle);
          const auto& else_value = func_vars_else.at(handle);
          func_vars[handle] = new ExprSelect(
            then_value->ty,
            cond,
            then_value,
            else_value);
        }

        if (!then_block->is<StmtNop>() || !else_block->is<StmtNop>()) {
          StmtRef branch = new StmtConditionalBranch(cond, then_block, else_block);
          out_stmts.emplace_back(branch);
        }

      }

    }

    if (out_stmts.empty()) {
      return new StmtNop;
    } else {
      return new StmtBlock(std::move(out_stmts));
    }
  }

};

struct CtrlflowStmt2ExprPass : public Pass {
  CtrlflowStmt2ExprPass() : Pass("ctrlflow-stmt2expr") {}
  virtual void apply(NodeRef& x) override final {
    CtrlflowStmt2ExprMutator v;
    x = v.mutate(x);
  }
};
static Pass* PASS = reg_pass<CtrlflowStmt2ExprPass>();
