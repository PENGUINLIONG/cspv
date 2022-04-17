#include "gft/log.hpp"
#include "pass/ranged-loop-elevation.hpp"
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct RangedLoopElevationVisitor : public StmtVisitor {
  struct Candidate {
    std::shared_ptr<Memory> func_var;
    std::shared_ptr<Expr> begin_expr;
    std::shared_ptr<Expr> stride_expr;
  };
  struct RangedLoop {
    std::shared_ptr<Memory> func_var;
    std::shared_ptr<Expr> begin_expr;
    std::shared_ptr<Expr> end_expr;
    std::shared_ptr<Expr> stride_expr;
  };

  // `nullptr` if the value is diverged.
  std::map<std::shared_ptr<Memory>, std::shared_ptr<Expr>> mem_value_map;

  std::vector<RangedLoop> ranged_loops;

  virtual void visit_stmt_(const StmtStore& x) override final {
    if (x.dst_ptr->cls != L_MEMORY_CLASS_FUNCTION_VARIABLE) { return; }
    log::info(dbg_print(*x.dst_ptr), " ", dbg_print(*x.value));
    mem_value_map.emplace(x.dst_ptr, x.value);
  }
  virtual void visit_stmt_(const StmtConditionalBranch& x) override final {
    auto mem_value_map2 = mem_value_map;
    visit_stmt(*x.then_block);
    mem_value_map = std::exchange(mem_value_map2, std::move(mem_value_map));
    visit_stmt(*x.else_block);
    for (const auto& pair : mem_value_map2) {
      auto it = mem_value_map.find(pair.first);
      if (it == mem_value_map.end()) {
        mem_value_map.emplace(pair);
      } else if (it->second != pair.second) {
        // Mark the memory content as diverged.
        it->second = nullptr;
      }
    }
  }
  virtual void visit_stmt_(const StmtLoop& x) override final {
    std::map<std::shared_ptr<Memory>, Candidate> candidates;
    visit_stmt_functor<StmtStore>(
      [&](const StmtStore& store) {
        visit_expr_functor<ExprLoad>(
          [&](const ExprLoad& load) {
            bool load_store_func_vars =
              load.src_ptr->is<MemoryFunctionVariable>() &&
              store.dst_ptr->is<MemoryFunctionVariable>();
            if (!load_store_func_vars) { return; }

            const auto& load_var = load.src_ptr->as<MemoryFunctionVariable>();
            const auto& store_var = store.dst_ptr->as<MemoryFunctionVariable>();
            if (load_var.handle != store_var.handle) { return; }

            if (!store.value->is<ExprAdd>()) { return; }
            const auto& value_expr = store.value->as<ExprAdd>();

            std::shared_ptr<Expr> stride_expr;
            if (value_expr.a.get() == &load) {
              stride_expr = value_expr.b;
            } else if (value_expr.b.get() == &load) {
              stride_expr = value_expr.a;
            } else { return; }

            auto it = mem_value_map.find(load.src_ptr);
            if (it == mem_value_map.end() || it->second == nullptr) { return; }

            Candidate candidate {};
            candidate.func_var = load.src_ptr;
            candidate.begin_expr = it->second;
            candidate.stride_expr = std::move(stride_expr);
            candidates.emplace(load.src_ptr, std::move(candidate));

          }, *store.value);
      }, *x.continue_block);

    for (const auto& stmt : x.body_block->as<StmtBlock>().stmts) {
      if (!stmt->is<StmtConditionalBranch>()) { continue; }

      const auto& cond_branch = stmt->as<StmtConditionalBranch>();
      const auto& else_block = cond_branch.else_block->as<StmtBlock>();
      if (!else_block.stmts.front()->is<StmtLoopMerge>()) { continue; }

      switch (cond_branch.cond->op) {
      case L_EXPR_OP_LT:
      {
        const auto& cond_expr = cond_branch.cond->as<ExprLt>();
        if (!cond_expr.a->is<ExprLoad>()) { continue; }

        auto it = candidates.find(cond_expr.a->as<ExprLoad>().src_ptr);
        if (it == candidates.end()) { continue; }

        RangedLoop ranged_loop {};
        ranged_loop.func_var = std::move(it->second.func_var);
        ranged_loop.begin_expr = std::move(it->second.begin_expr);
        ranged_loop.end_expr = cond_expr.b;
        ranged_loop.stride_expr = std::move(it->second.stride_expr);
        ranged_loops.emplace_back(std::move(ranged_loop));
        break;
      }
      default: continue;
      }
    }

    visit_stmt(*x.body_block);
  }
};

void ranged_loop_elevation(const std::shared_ptr<Stmt>& x) {
  RangedLoopElevationVisitor visitor;
  visitor.visit_stmt(*x);
}
