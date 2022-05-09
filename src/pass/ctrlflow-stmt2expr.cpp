#include <algorithm>
#include <set>
#include <iterator>
#include "pass/pass.hpp"
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct FunctionVariableRecord {
  MemoryFunctionVariableRef func_var;
  uint32_t scope_lv;
  ExprRef value;
};
struct ScopeRecord {
  uint32_t scope_lv;
  // Function variables that are overriden by new assignments within the scope.
  // The changes should propagate to the outer scope so that the mutator stop
  // using the out-dated values.
  std::vector<MemoryRef> overriden_func_vars;
  std::vector<FunctionVariableRecord> func_vars;
  
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
  // Returns whether the function variable state specified by the outer scope
  // has been overriden.
  bool set_func_var(const MemoryFunctionVariableRef& func_var, const ExprRef& value) {
    FunctionVariableRecord* record = nullptr;
    if (try_get_func_var(func_var, record)) {
      bool is_overriden = scope_lv > record->scope_lv;
      record->value = value;
      record->scope_lv = scope_lv;
      if (is_overriden) {
        // Notify the outer scope to fix value states.
        overriden_func_vars.emplace_back(func_var);
        return true;
      }
    } else {
      FunctionVariableRecord record {};
      record.func_var = func_var;
      record.scope_lv = scope_lv;
      record.value = value;
      func_vars.emplace_back(std::move(record));
    }
    return false;
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

  void push_scope() {
    ScopeRecord record {};
    record.scope_lv = scope_stack.back().scope_lv + 1;
    record.func_vars = scope_stack.back().func_vars;
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
        if (func_var_record->scope_lv == get_scope().scope_lv) {
          // This value is made within the scope. In terms of loops, the value is assigned in this current iteration.
          return func_var_record->value;
        } else {
          // The variable is created by an inner scope? It should not happen and these inner variables should be carried out by algorithms.
          assert(func_var_record->scope_lv < get_scope().scope_lv);
          return x;
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

  virtual StmtRef mutate_stmt_(StmtLoopRef x) override final {
    std::vector<StmtRef> stmts;

    push_scope();
    StmtBlockRef body_block = mutate_stmt(x->body_block).as<StmtBlock>();
    ScopeRecord body_scope = pop_scope();
    for (auto func_var : body_scope.overriden_func_vars) {
      ExprRef outer_value = get_scope().get_func_var(func_var).value;
      stmts.emplace_back(new StmtStore(func_var, outer_value));
    }
    body_block = body_block;

    push_scope();
    StmtBlockRef continue_block = mutate_stmt(x->continue_block).as<StmtBlock>();
    ScopeRecord continue_scope = pop_scope();
    for (auto func_var : continue_scope.overriden_func_vars) {
      ExprRef outer_value = get_scope().get_func_var(func_var).value;
      stmts.emplace_back(new StmtStore(func_var, outer_value));
    }
    continue_block = continue_block;

    stmts.emplace_back(new StmtLoop(body_block, continue_block, x->handle));
    return new StmtBlock(std::move(stmts));
  }

  virtual StmtRef mutate_stmt_(StmtStoreRef x) override final {
    if (!x->dst_ptr->is<MemoryFunctionVariable>()) {
      return Mutator::mutate_stmt_(x);
    }

    auto dst_ptr = mutate_mem(x->dst_ptr);
    auto value = mutate_expr(x->value);

    if (get_scope().set_func_var(dst_ptr, value)) {
      return new StmtStore(dst_ptr, value);
    } else {
      return new StmtBlock({});
    }
  }

  virtual StmtRef mutate_stmt_(StmtBlockRef x) override final {
    std::vector<StmtRef> out_stmts;
    for (StmtRef stmt : x->stmts) {
      if (stmt->is<StmtConditionalBranch>()) {
        StmtConditionalBranchRef stmt2 = stmt;
        ExprRef cond = stmt2->cond;

        auto func_vars2 = scope_stack.back().func_vars;
        StmtBlockRef then_block = mutate_stmt(stmt2->then_block);
        auto func_vars_then = std::move(scope_stack.back().func_vars);

        scope_stack.back().func_vars = std::move(func_vars2);
        StmtBlockRef else_block = mutate_stmt(stmt2->else_block);
        auto func_vars_else = std::move(scope_stack.back().func_vars);

        // Mapping from handle to function variable. A variable must exist in
        // both branch to be picked into the outer scope.
        std::vector<MemoryFunctionVariableRef> common_vars;
        for (const auto& then_var : func_vars_then) {
          auto it = std::find_if(
            func_vars_else.begin(),
            func_vars_else.end(),
            [&](const FunctionVariableRecord& else_var) {
              return then_var.func_var->structured_eq(else_var.func_var) &&
                !then_var.value->structured_eq(else_var.value);
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

        if (!then_block->stmts.empty() || !else_block->stmts.empty()) {
          StmtRef branch = new StmtConditionalBranch(cond, then_block, else_block);
          out_stmts.emplace_back(branch);
        }

      } else {
        out_stmts.emplace_back(mutate_stmt(stmt));
      }
    }

    StmtRef stmt = new StmtBlock(std::move(out_stmts));
    return stmt;
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
