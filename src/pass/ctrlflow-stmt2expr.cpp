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
  uint32_t scope_lv;
  ExprRef value;
};
struct ScopeRecord {
  uint32_t scope_lv;
  std::vector<StmtRef> prelude;
  std::vector<FunctionVariableRecord> func_vars;
  bool is_within_loop;
  
  bool try_get_func_var(
    const MemoryFunctionVariableRef& func_var,
    FunctionVariableRecord*& out
  ) {
    auto it = std::find_if(
      func_vars.begin(),
      func_vars.end(),
      [&](const FunctionVariableRecord& else_var) {
        return func_var->structured_eq(else_var.func_var);
      });
    if (it != func_vars.end()) {
      out = &*it;
      return true;
    } else {
      return false;
    }
  }
  FunctionVariableRecord& get_func_var(const MemoryFunctionVariableRef& func_var) {
    FunctionVariableRecord* record = nullptr;
    assert(try_get_func_var(func_var, record));
    return *record;
  }
  void set_func_var(const MemoryFunctionVariableRef& func_var, const ExprRef& value) {
    FunctionVariableRecord* record = nullptr;
    if (try_get_func_var(func_var, record)) {
      record->value = value;
    } else {
      FunctionVariableRecord record {};
      record.func_var = func_var;
      record.value = value;
      func_vars.emplace_back(std::move(record));
    }
  }
  void remove_func_var(const MemoryFunctionVariableRef& func_var) {
    auto it = std::find_if(
      func_vars.begin(),
      func_vars.end(),
      [&](const FunctionVariableRecord& else_var) {
        return func_var->structured_eq(else_var.func_var);
      });
    if (it != func_vars.end()) {
      func_vars.erase(it);
    }
  }
};

struct CtrlflowStmt2ExprMutator : public Mutator {

  std::vector<ScopeRecord> scope_stack { {} };
  ScopeRecord& get_scope() {
    return scope_stack.back();
  }

  void push_scope(bool is_within_loop) {
    ScopeRecord record {};
    record.scope_lv = scope_stack.back().scope_lv + 1;
    record.func_vars = scope_stack.back().func_vars;
    record.is_within_loop = is_within_loop || scope_stack.back().is_within_loop;
    scope_stack.emplace_back(std::move(record));
  }
  ScopeRecord pop_scope() {
    auto record = std::move(scope_stack.back());
    scope_stack.pop_back();
    return record;
  }




  virtual ExprRef mutate_expr_(ExprLoadRef x) override final {
    if (x->src_ptr->is<MemoryFunctionVariable>()) {
      MemoryFunctionVariableRef src_ptr = x->src_ptr;
      FunctionVariableRecord* func_var_record = nullptr;
      if (get_scope().try_get_func_var(src_ptr, func_var_record)) {
        if (func_var_record->scope_lv < get_scope().scope_lv) {
          // The variable got its value from an outer scope. In case of loops, the value might change over time if it's referenced within iterations.
          get_scope().prelude.emplace_back(new StmtStore(src_ptr, func_var_record->value));
          get_scope().remove_func_var(src_ptr);
          return x;
        } else if (func_var_record->scope_lv == get_scope().scope_lv) {
          // This value is make within the scope. In terms of loops, the value is assigned in this current iteration.
          return func_var_record->value;
        } else {
          // The variable is created by an inner scope? It should not happen and these inner variables should be carried out by algorithms.
          unreachable();
        }
      } else {
        // The only case a function variable cannot be found is that, it's been removed because its created by an outer scope and the variable has been loaded before.
        // Otherwise the variable is never assigned even in the original program, and there is nothing we can help with.
        return x;
      }
    } else {
      return x;
    }
  }

  virtual StmtRef mutate_stmt_(StmtConditionalLoopRef x) override final {
    push_scope(true);
    x = Mutator::mutate_stmt_(x);
    ScopeRecord scope = pop_scope();
    if (scope.prelude.empty()) {
      return x;
    } else {
      scope.prelude.emplace_back(x);
      return new StmtBlock(std::move(scope.prelude));
    }
  }

  virtual StmtRef mutate_stmt_(StmtBlockRef x) override final {
    std::vector<StmtRef> out_stmts;
    for (StmtRef stmt : x->stmts) {
      if (stmt->is<StmtStore>()) {
        StmtStoreRef stmt2 = mutate_stmt(stmt);
        if (stmt2->dst_ptr->is<MemoryFunctionVariable>()) {
          get_scope().set_func_var(stmt2->dst_ptr, mutate_expr(stmt2->value));
        } else {
          out_stmts.emplace_back(stmt);
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
          out_stmts.emplace_back(mutate_stmt(branch));
        }

      } else {
        out_stmts.emplace_back(mutate_stmt(stmt));
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
