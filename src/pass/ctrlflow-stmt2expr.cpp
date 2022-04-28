#include <algorithm>
#include <set>
#include <iterator>
#include "pass/pass.hpp"
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

using namespace liong;

bool func_var_eq(
  const MemoryFunctionVariableRef& a,
  const MemoryFunctionVariableRef& b
) {
  return a->structured_eq(b);
}

struct FunctionVariableRecord {
  MemoryFunctionVariableRef func_var;
  size_t scope_idx; // Index in `scope_stack`.
  ExprRef value;
};
struct ScopeRecord {
  size_t idx;
  std::set<MemoryFunctionVariableRef> stored_vars;
  std::vector<FunctionVariableRecord> func_vars;

  FunctionVariableRecord& get_func_var(const MemoryFunctionVariableRef& func_var) {
    auto it = std::find_if(
      func_vars.begin(),
      func_vars.end(),
      [&](const FunctionVariableRecord& else_var) {
        return func_var->structured_eq(else_var.func_var);
      });
    assert(it != func_vars.end());
    return *it;
  }
  void set_func_var(const MemoryFunctionVariableRef& func_var, const ExprRef& value) {
    auto it = std::find_if(
      func_vars.begin(),
      func_vars.end(),
      [&](const FunctionVariableRecord& else_var) {
        return func_var->structured_eq(else_var.func_var);
      });
    if (it == func_vars.end()) {
      FunctionVariableRecord record {};
      record.func_var = func_var;
      record.scope_idx = idx;
      record.value = value;
      func_vars.emplace_back(std::move(record));
    } else {
      it->value = value;
    }
  }
};

struct CtrlflowStmt2ExprMutator : public Mutator {
  std::vector<ScopeRecord> scope_stack { { 1 } };

  ScopeRecord& get_scope() {
    return scope_stack.back();
  }

  void push_scope() {
    ScopeRecord record {};
    record.idx = scope_stack.size() + 1;
    record.stored_vars = scope_stack.back().stored_vars;
    scope_stack.emplace_back(std::move(record));
  }
  std::vector<StmtRef> pop_scope() {
    std::vector<StmtRef> out;
    for (const auto& func_var : scope_stack.back().stored_vars) {
      const FunctionVariableRecord& func_var_record = get_scope().get_func_var(func_var);
      MemoryFunctionVariableRef func_var = func_var_record.func_var;
      StmtRef stmt = new StmtStore(func_var, func_var_record.value);
    }
    scope_stack.pop_back();
    return out;
  }




  virtual ExprRef mutate_expr_(ExprLoadRef x) override final {
    if (x->src_ptr->is<MemoryFunctionVariable>()) {
      MemoryFunctionVariableRef src_ptr = x->src_ptr;
      const auto& func_var_record = get_scope().get_func_var(src_ptr);

      if (scope_stack.size() != func_var_record.scope_idx) {
        scope_stack.back().stored_vars.insert(src_ptr);
        return x;
      } else {
        return func_var_record.value;
      }
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
          get_scope().set_func_var(stmt2->dst_ptr, mutate_expr(stmt2->value));
        } else {
          out_stmts.emplace_back(mutate_stmt(stmt));
        }
      } else if (stmt->is<StmtConditionalBranch>()) {
        StmtConditionalBranchRef stmt2 = stmt;
        ExprRef cond = mutate_expr(stmt2->cond);

        auto func_vars2 = scope_stack.back().func_vars;
        StmtRef then_block = mutate_stmt(stmt2->then_block);
        auto func_vars_then = std::move(scope_stack.back().func_vars);

        scope_stack.back().func_vars = std::move(func_vars2);
        StmtRef else_block = mutate_stmt(stmt2->else_block);
        auto func_vars_else = std::move(scope_stack.back().func_vars);

        // Mapping from handle to function variable. A variable must exist in
        // both branch to be picked into the outer scope.
        std::vector<MemoryFunctionVariableRef> common_vars;
        for (const auto& then_var : func_vars_then) {
          auto it = std::find_if(
            func_vars_else.begin(),
            func_vars_else.end(),
            [&](const FunctionVariableRecord& else_var) {
              return then_var.func_var->structured_eq(else_var.func_var);
            });
          if (it != func_vars_else.end()) {
            ExprRef expr = new ExprSelect(
              then_var.value->ty,
              cond,
              then_var.value,
              it->value);
            get_scope().set_func_var(then_var.func_var, expr);
          }
        }

        if (!then_block->is<StmtNop>() || !else_block->is<StmtNop>()) {
          StmtRef branch = new StmtConditionalBranch(cond, then_block, else_block);
          out_stmts.emplace_back(mutate_stmt_(branch));
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
