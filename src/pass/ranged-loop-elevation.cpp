#include <set>
#include "gft/log.hpp"
#include "pass/pass.hpp"
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct RangedLoopElevationMutator : public Mutator {
  struct Candidate {
    MemoryRef func_var;
    ExprRef begin_expr;
    ExprRef stride_expr;
    // If the candidate is acknowledged, the value will directly be assigned
    // with `end_expr`.
    StmtStoreRef init_store_stmt;
  };

  struct FunctionVariableHistory {
    ExprRef value;
    StmtStoreRef store_stmt;
  };

  // `nullptr` if the value is diverged.
  std::map<MemoryRef, FunctionVariableHistory> mem_value_map;

  virtual ExprRef mutate_expr_(ExprLoadRef& x) override final {
    if (x->src_ptr->is<MemoryFunctionVariable>()) {
      mem_value_map.erase(x->src_ptr);
    }
    return Mutator::mutate_expr_(x);
  }

  virtual StmtRef mutate_stmt_(StmtStoreRef& x) override final {
    if (x->dst_ptr->cls != L_MEMORY_CLASS_FUNCTION_VARIABLE) {
      return x.as<Stmt>();
    }
    log::info(dbg_print(x->dst_ptr.as<Node>()), " ", dbg_print(x->value.as<Node>()));

    FunctionVariableHistory hist {};
    hist.value = x->value;
    hist.store_stmt = x;
    mem_value_map.emplace(x->dst_ptr, std::move(hist));
    return Mutator::mutate_stmt_(x);
  }
  virtual StmtRef mutate_stmt_(StmtConditionalBranchRef& x) override final {
    auto mem_value_map2 = mem_value_map;
    x->then_block = mutate_stmt(x->then_block);
    mem_value_map = std::exchange(mem_value_map2, std::move(mem_value_map));
    x->else_block = mutate_stmt(x->else_block);
    for (const auto& pair : mem_value_map2) {
      auto it = mem_value_map.find(pair.first);
      if (it == mem_value_map.end()) {
        mem_value_map.emplace(pair);
      } else if (it->second.value != pair.second.value) {
        // Mark the memory content as diverged.
        mem_value_map.erase(it);
      }
    }
    return x.as<Stmt>();
  }
  virtual StmtRef mutate_stmt_(StmtLoopRef& x) override final {
    std::map<MemoryRef, Candidate> candidates;
    visit_stmt_functor<StmtStore>(
      [this, &candidates](const StmtStoreRef& store) {
        visit_expr_functor<ExprLoad>(
          [this, &candidates, &store](const ExprLoadRef& load) {
            bool load_store_func_vars =
              load->src_ptr->is<MemoryFunctionVariable>() &&
              store->dst_ptr->is<MemoryFunctionVariable>();
            if (!load_store_func_vars) { return; }

            const auto& load_var = load->src_ptr->as<MemoryFunctionVariable>();
            const auto& store_var = store->dst_ptr->as<MemoryFunctionVariable>();
            if (load_var.handle != store_var.handle) { return; }

            if (!store->value->is<ExprAdd>()) { return; }
            const auto& value_expr = store->value->as<ExprAdd>();

            ExprRef stride_expr;
            if (value_expr.a == load) {
              stride_expr = value_expr.b;
            } else if (value_expr.b == load) {
              stride_expr = value_expr.a;
            } else { return; }

            auto it = mem_value_map.find(load->src_ptr);
            if (it == mem_value_map.end()) { return; }

            Candidate candidate {};
            candidate.func_var = load->src_ptr;
            candidate.begin_expr = it->second.value;
            candidate.stride_expr = std::move(stride_expr);
            candidate.init_store_stmt = it->second.store_stmt;
            candidates.emplace(load->src_ptr, std::move(candidate));

          }, store->value);
      }, x->continue_block);

    const auto& cond_branch = x->body_block->as<StmtConditionalBranch>();
    if (cond_branch.else_block->is<StmtLoopMerge>()) {
      switch (cond_branch.cond->op) {
      case L_EXPR_OP_LT:
      {
        const auto& cond_expr = cond_branch.cond->as<ExprLt>();
        if (!cond_expr.a->is<ExprLoad>()) { break; }

        auto it = candidates.find(cond_expr.a->as<ExprLoad>().src_ptr);
        if (it == candidates.end()) { break; }

        auto& candidate = it->second;
        // FIXME: (penguinliong) Use itervars to replace variables inside loops,
        // so this change in initial store value won't interfered the loop
        // content.
        candidate.init_store_stmt->value = cond_expr.b;
        return StmtRef(new StmtRangedLoop(cond_branch.then_block,
          candidate.func_var, candidate.begin_expr, cond_expr.b,
          candidate.stride_expr));
      }
      default: break;
      }
    }

    return Mutator::mutate_stmt_(x);
  }

};

void ranged_loop_elevation(StmtRef& x) {
  RangedLoopElevationMutator mutator;
  x = mutator.mutate_stmt(x);
}
