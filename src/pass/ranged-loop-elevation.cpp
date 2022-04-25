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

    // Ranged loop has an only itervar mutated in the continue block.
    TypePatternCaptureRef func_var_ty_pat = new TypePatternCapture;
    MemoryPatternCaptureRef func_var_pat = new MemoryPatternCapture(func_var_ty_pat, {});
    ExprPatternCaptureRef stride_pat = new ExprPatternCapture(func_var_ty_pat);
    ExprPatternCaptureRef end_pat = new ExprPatternCapture(func_var_ty_pat);

    StmtRef update_pat = new StmtStore(
      func_var_pat,
      new ExprPatternBinaryOp(
        func_var_ty_pat,
        {},
        new ExprLoad(func_var_ty_pat, func_var_pat),
        stride_pat
      )
    );
    StmtRef cond_pat = new StmtPatternHead(
      new StmtConditionalBranch(
        new ExprNot(
          new TypeBool,
          new ExprPatternBinaryOp(
            new TypeBool,
            {},
            new ExprLoad(func_var_ty_pat, func_var_pat),
            end_pat
          )
        ),
        new StmtLoopMerge(x->handle),
        new StmtNop
      )
    );

    if (!match_pattern(update_pat, x->continue_block)) { return x; }

    auto func_var = func_var_pat->captured;
    if (!func_var->is<MemoryFunctionVariable>()) { return x; }
    auto func_var_ty = func_var_ty_pat->captured;

    auto init_value = mem_value_map.find(func_var);
    if (init_value == mem_value_map.end()) { return x; }
    auto begin_expr = init_value->second;

    if (!match_pattern(cond_pat, x->body_block)) { return x; }
    x->continue_block = mutate_stmt(x->continue_block);
    x->body_block = mutate_stmt(x->body_block);


    auto stride_expr = stride_pat->captured;
    auto end_expr = end_pat->captured;

    MemoryRef itervar = new MemoryIterationVariable(
      func_var_ty, {}, begin_expr, end_expr, stride_expr);
    StmtRef new_body = x->body_block->is<StmtBlock>() ?
      StmtRef(new StmtBlock({ x->body_block.as<StmtBlock>()->stmts.begin() + 1, x->body_block.as<StmtBlock>()->stmts.end() })) :
      StmtRef(new StmtNop);

    return StmtRef(new StmtBlock({
      new StmtRangedLoop(new_body, itervar),
      new StmtStore(func_var, end_expr),
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
