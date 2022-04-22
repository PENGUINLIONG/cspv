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
  };

  // `nullptr` if the value is diverged.
  std::map<MemoryRef, ExprRef> mem_value_map;
  std::map<MemoryFunctionVariableRef, MemoryIterationVariableRef> itervar_map;

  virtual ExprRef mutate_expr_(ExprLoadRef x) override final {
    if (x->src_ptr->is<MemoryFunctionVariable>()) {
      mem_value_map.erase(x->src_ptr);

      auto it = itervar_map.find(x->src_ptr);
      if (it != itervar_map.end()) {
        x->src_ptr = it->second;
      }
    }
    return Mutator::mutate_expr_(x);
  }

  virtual StmtRef mutate_stmt_(StmtStoreRef x) override final {
    if (x->dst_ptr->is<MemoryFunctionVariable>()) {
      mem_value_map.emplace(x->dst_ptr, x->value);
    }
    return Mutator::mutate_stmt_(x);
  }
  virtual StmtRef mutate_stmt_(StmtConditionalBranchRef x) override final {
    auto mem_value_map2 = mem_value_map;
    x->then_block = mutate_stmt(x->then_block);
    mem_value_map = std::exchange(mem_value_map2, std::move(mem_value_map));
    x->else_block = mutate_stmt(x->else_block);
    for (const auto& pair : mem_value_map2) {
      auto it = mem_value_map.find(pair.first);
      if (it == mem_value_map.end()) {
        mem_value_map.emplace(pair);
      } else if (it->second != pair.second) {
        // Mark the memory content as diverged.
        mem_value_map.erase(it);
      }
    }
    return x.as<Stmt>();
  }
  virtual StmtRef mutate_stmt_(StmtLoopRef x) override final {
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
            candidate.begin_expr = it->second;
            candidate.stride_expr = std::move(stride_expr);
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

        auto begin_expr = candidate.begin_expr;
        auto end_expr = cond_expr.b;
        auto stride_expr = candidate.stride_expr;

        auto itervar = MemoryRef(new MemoryIterationVariable(
          candidate.func_var->ty, {}, begin_expr, end_expr, stride_expr));

        itervar_map.emplace(candidate.func_var, itervar);
        auto then_block = mutate(cond_branch.then_block);
        itervar_map.erase(candidate.func_var);
        mem_value_map[candidate.func_var] = end_expr;

        return StmtRef(new StmtBlock({
          new StmtRangedLoop(then_block, itervar),
          new StmtStore(candidate.func_var, end_expr),
        }));
      }
      default: break;
      }
    }

    return Mutator::mutate_stmt_(x);
  }

};

struct RangedLoopElevationPass : public Pass {
  RangedLoopElevationPass() : Pass("ranged-loop-elevation") {}
  virtual void apply(NodeRef<Node>& x) override final {
    RangedLoopElevationMutator v;
    x = v.mutate(x);
  }
};
static Pass* PASS = reg_pass<RangedLoopElevationPass>();
