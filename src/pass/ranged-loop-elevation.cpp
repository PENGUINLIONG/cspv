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
    return x;
  }
  virtual StmtRef mutate_stmt_(StmtLoopRef x) override final {
    x->body_block = mutate_stmt(x->body_block);

    // Ranged loop has an only itervar mutated in the continue block.
    if (!x->continue_block->is<StmtStore>()) { return x; }
    auto store = x->continue_block->as<StmtStore>();
    auto update_expr = store.value;
    auto stride_expr = update_expr.as<ExprAdd>()->b;

    if (!store.dst_ptr->is<MemoryFunctionVariable>()) { return x; }
    auto init_value = mem_value_map.find(store.dst_ptr);
    if (init_value == mem_value_map.end()) { return x; }
    auto begin_expr = init_value->second;

    x->continue_block = mutate_stmt(x->continue_block);

    auto& body_head = get_head_stmt(x->body_block);
    if (!body_head->is<StmtConditionalBranch>()) { return x; }
    auto branch = body_head->as<StmtConditionalBranch>();
    // Match simple breaks.
    if (!branch.then_block->is<StmtLoopMerge>() || !branch.else_block->is<StmtNop>()) { return x; }
    auto cond = branch.cond;
    auto end_expr = cond.as<ExprNot>()->a.as<ExprLt>()->b;
    mem_value_map[store.dst_ptr] = end_expr;

    MemoryRef itervar = new MemoryIterationVariable(
      store.dst_ptr->ty, {}, begin_expr, end_expr, stride_expr);
    StmtRef new_body = x->body_block->is<StmtBlock>() ?
      StmtRef(new StmtBlock({ x->body_block.as<StmtBlock>()->stmts.begin() + 1, x->body_block.as<StmtBlock>()->stmts.end() })) :
      StmtRef(new StmtNop);

    return StmtRef(new StmtBlock({
      new StmtRangedLoop(new_body, itervar),
      new StmtStore(store.dst_ptr, end_expr),
    }));
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
