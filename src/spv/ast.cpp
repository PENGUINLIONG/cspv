#include "spv/ast.hpp"

using namespace liong;

struct SelectionMergeState {
  spv::SelectionControlMask sel_ctrl;
};
struct LoopMergeState {
  spv::LoopControlMask loop_ctrl;
  InstructionRef continue_target_label;
};
struct MergeState {
  InstructionRef merge_target_label;
  InstructionRef head_label;
  std::unique_ptr<SelectionMergeState> sel;
  std::unique_ptr<LoopMergeState> loop;
};

struct ControlFlowGraphParser {
  const SpirvModule& mod;
  const SpirvFunction& func;
  std::vector<MergeState> merge_states;

  ControlFlowGraphParser(const SpirvModule& mod, const SpirvFunction& func) :
    mod(mod), func(func), merge_states()
  {
    merge_states.reserve(1); // Has at most 1 element.
  }

  inline const InstructionRef& lookup_instr(spv::Id id) const {
    return mod.abstr.id2instr_map.at(id);
  }



  void push_merge_state(
    const InstructionRef& head_label,
    const InstructionRef& merge_instr
  ) {
    if (merge_instr.op() == spv::Op::OpSelectionMerge) {
      auto e = merge_instr.extract_params();
      const InstructionRef& merge_target_label = lookup_instr(e.read_id());
      spv::SelectionControlMask sel_ctrl =
        e.read_u32_as<spv::SelectionControlMask>();

      SelectionMergeState sel {};
      sel.sel_ctrl = sel_ctrl;

      MergeState merge_state {};
      merge_state.head_label = head_label;
      merge_state.merge_target_label = merge_target_label;
      merge_state.sel = std::make_unique<SelectionMergeState>(std::move(sel));
      merge_states.emplace_back(std::move(merge_state));

    } else if (merge_instr.op() == spv::Op::OpLoopMerge) {
      auto e = merge_instr.extract_params();
      const InstructionRef& merge_target_label = lookup_instr(e.read_id());
      const InstructionRef& continue_target_label = lookup_instr(e.read_id());
      spv::LoopControlMask loop_ctrl = e.read_u32_as<spv::LoopControlMask>();

      LoopMergeState loop {};
      loop.continue_target_label = continue_target_label;
      loop.loop_ctrl = loop_ctrl;

      MergeState merge_state {};
      merge_state.head_label = head_label;
      merge_state.merge_target_label = merge_target_label;
      merge_state.loop = std::make_unique<LoopMergeState>(std::move(loop));
      merge_states.emplace_back(std::move(merge_state));

    } else {
      assert(merge_instr.op() == spv::Op::OpNop,
        "unexpected merge instruction");
    }

    //assert(merge_states.size() == 1);
  }
  void pop_merge_state() {
    //assert(merge_states.size() == 1);
    merge_states.pop_back();
  }

  std::unique_ptr<ControlFlow> parse_branch_conditional(const Block& block) {
    assert(!merge_states.empty());
    MergeState& merge_state = merge_states.back();

    auto e = block.term.extract_params();
    const InstructionRef& cond = lookup_instr(e.read_id());
    const InstructionRef& then_target_label = lookup_instr(e.read_id());
    const InstructionRef& else_target_label = lookup_instr(e.read_id());

    std::unique_ptr<ControlFlow> out {};
    InstructionRef merge_target_label = merge_state.merge_target_label;
    if (merge_state.sel) {
      std::shared_ptr<Expr> cond_expr = parse_expr(mod, cond);

      std::vector<Branch> branches;
      branches.reserve(2);
      if (then_target_label != merge_state.merge_target_label) {
        Branch then_branch {};
        then_branch.branch_ty = L_BRANCH_TYPE_CONDITION_THEN;
        then_branch.cond = cond_expr;
        then_branch.ctrl_flow = parse(then_target_label);

        branches.emplace_back(std::move(then_branch));
      }
      if (else_target_label != merge_state.merge_target_label) {
        Branch else_branch {};
        else_branch.branch_ty = L_BRANCH_TYPE_CONDITION_ELSE;
        else_branch.cond = cond_expr;
        else_branch.ctrl_flow = parse(else_target_label);

        branches.emplace_back(std::move(else_branch));
      }

      pop_merge_state();
      auto next = parse(merge_target_label);
      return std::make_unique<ControlFlowSelect>(block.label, std::move(next), std::move(branches));

    } else if (merge_state.loop) {
      assert(else_target_label == merge_state.merge_target_label);

      Branch body_branch {};
      body_branch.branch_ty = L_BRANCH_TYPE_CONDITION_THEN;
      body_branch.cond = parse_expr(mod, cond);
      body_branch.ctrl_flow = parse(then_target_label);

      pop_merge_state();
      auto next = parse(merge_target_label);
      return std::make_unique<ControlFlowLoop>(block.label, std::move(next), std::move(body_branch));

    } else {
      unreachable();
    }
  }
  std::unique_ptr<ControlFlow> parse_branch(const Block& block) {
    auto e = block.term.extract_params();
    auto next = parse(lookup_instr(e.read_id()));
    return std::make_unique<ControlFlowJump>(block.label, std::move(next));
  }
  std::unique_ptr<ControlFlow> parse_return(const Block& block) {
    return std::make_unique<ControlFlowReturn>(block.label, nullptr);
  }

  std::unique_ptr<ControlFlow> parse_executable(const Block& block, std::unique_ptr<ControlFlow>&& inner) {
    if (block.instrs.empty()) {
      return std::forward<std::unique_ptr<ControlFlow>>(inner);
    }

    std::vector<std::shared_ptr<Stmt>> stmts;
    for (const auto& instr : block.instrs) {
      std::shared_ptr<Stmt> stmt = parse_stmt(mod, instr);
      if (stmt != nullptr) {
        stmts.emplace_back(std::move(stmt));
      }
    }
    return std::make_unique<ControlFlowExecutable>(block.label,
      std::forward<std::unique_ptr<ControlFlow>>(inner), std::move(stmts));
  }

  std::unique_ptr<ControlFlow> parse_block(const Block& block) {
    if (!merge_states.empty()) {
      for (auto it = merge_states.rbegin(); it != merge_states.rend(); ++it) {
        const MergeState& merge_state = *it;
        if (merge_state.loop) {
          if (block.label == merge_state.merge_target_label) {
            return std::make_unique<ControlFlowMergeLoop>(block.label);
          } else if (merge_state.head_label == block.label) {
            return std::make_unique<ControlFlowBackEdge>(block.label);
          }
        } else if (merge_state.sel) {
          if (block.label == merge_state.merge_target_label) {
            return std::make_unique<ControlFlowMergeSelect>(block.label);
          }
        } else {
          unreachable();
        }
      }
    }

    if (block.merge != nullptr) {
      push_merge_state(block.label, block.merge);
    }

    std::unique_ptr<ControlFlow> out;
    switch (block.term.op()) {
    case spv::Op::OpBranchConditional:
      out = parse_branch_conditional(block);
      break;
    case spv::Op::OpBranch:
      out = parse_branch(block);
      break;
    case spv::Op::OpReturn:
      out = parse_return(block);
      break;
    default: unreachable();
    }

    out = parse_executable(block, std::move(out));
    return out;
  }

  std::unique_ptr<ControlFlow> parse(const InstructionRef& label) {
    if (label == nullptr) { return nullptr; }

    const Block& block = func.blocks.at(label);
    return parse_block(block);
  }
};

std::map<std::string, std::unique_ptr<ControlFlow>> extract_entry_points(const SpirvModule& mod) {
  std::map<std::string, std::unique_ptr<ControlFlow>> out {};

  for (const auto& pair : mod.entry_points) {
    const auto& entry_point = mod.funcs.at(pair.second.func);
    std::unique_ptr<ControlFlow> ctrl_flow =
      ControlFlowGraphParser(mod, entry_point).parse(entry_point.entry_label);
    out.emplace(pair.second.name, std::move(ctrl_flow));
  }

  return out;
}
