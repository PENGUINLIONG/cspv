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



  std::shared_ptr<Type> parse_ty(const InstructionRef& instr) {
    spv::Op op = instr.op();
    if (op == spv::Op::OpTypeVoid) {
      return std::shared_ptr<Type>(new TypeVoid);
    } else if (op == spv::Op::OpTypeBool) {
      return std::shared_ptr<Type>(new TypeBool);
    } else if (op == spv::Op::OpTypeInt) {
      auto e = instr.extract_params();
      uint32_t nbit = e.read_u32();
      bool is_signed = e.read_bool();
      return std::shared_ptr<Type>(new TypeInt(nbit, is_signed));
    } else if (op == spv::Op::OpTypeFloat) {
      auto e = instr.extract_params();
      uint32_t nbit = e.read_u32();
      return std::shared_ptr<Type>(new TypeFloat(nbit));
    } else if (op == spv::Op::OpTypeStruct) {
      auto e = instr.extract_params();
      std::vector<std::shared_ptr<Type>> members;
      while (e) {
        members.emplace_back(parse_ty(e.read_id()));
      }
      return std::shared_ptr<Type>(new TypeStruct(std::move(members)));
    } else if (op == spv::Op::OpTypePointer) {
      auto e = instr.extract_params();
      spv::StorageClass storage_cls = e.read_u32_as<spv::StorageClass>();
      auto inner = parse_ty(e.read_id());
      return std::shared_ptr<Type>(new TypePointer(inner));
    } else {
      panic("unsupported type");
    }
  }
  std::shared_ptr<Type> parse_ty(spv::Id id) {
    return parse_ty(lookup_instr(id));
  }

  std::shared_ptr<Memory> parse_mem(const InstructionRef& ptr) {
    std::shared_ptr<Memory> out;

    auto op = ptr.op();
    if (op == spv::Op::OpVariable) {
      auto ptr_ty = parse_ty(ptr.result_ty_id());
      assert(ptr_ty->cls == L_TYPE_CLASS_POINTER);
      auto var_ty = ((const TypePointer*)ptr_ty.get())->inner;

      auto e = ptr.extract_params();
      spv::StorageClass store_cls = e.read_u32_as<spv::StorageClass>();
      // Merely function vairables.
      if (store_cls == spv::StorageClass::Function) {
        return std::shared_ptr<Memory>(new MemoryFunctionVariable(var_ty, {}, ptr.inner));
      }

      // Descriptor resources.
      if (mod.has_deco(spv::Decoration::BufferBlock, ptr)) {
        store_cls = spv::StorageClass::StorageBuffer;
      }
      uint32_t binding = mod.get_deco_u32(spv::Decoration::Binding, ptr);
      uint32_t set = mod.get_deco_u32(spv::Decoration::DescriptorSet, ptr);
      if (store_cls == spv::StorageClass::Uniform) {
        return std::shared_ptr<Memory>(new MemoryUniformBuffer(var_ty, {}, binding, set));
      } else if (store_cls == spv::StorageClass::StorageBuffer) {
        return std::shared_ptr<Memory>(new MemoryStorageBuffer(var_ty, {}, binding, set));
      } else {
        panic("unsupported memory allocation");
      }

    } else if (op == spv::Op::OpAccessChain) {
      auto ptr_ty = parse_ty(ptr.result_ty_id());
      assert(ptr_ty->cls == L_TYPE_CLASS_POINTER);
      auto ty = ((const TypePointer*)ptr_ty.get())->inner;

      auto e = ptr.extract_params();
      auto base = parse_mem(e.read_id());
      AccessChain ac = base->ac;
      while (e) {
        ac.idxs.emplace_back(parse_expr(e.read_id()));
      }

      if (base->cls == L_MEMORY_CLASS_FUNCTION_VARIABLE) {
        const auto& base2 = *(const MemoryFunctionVariable*)base.get();
        return std::shared_ptr<Memory>(new MemoryFunctionVariable(ty, std::move(ac), base2.handle));
      } else if (base->cls == L_MEMORY_CLASS_UNIFORM_BUFFER) {
        const auto& base2 = *(const MemoryUniformBuffer*)base.get();
        return std::shared_ptr<Memory>(new MemoryUniformBuffer(ty, std::move(ac), base2.binding, base2.set));
      } else if (base->cls == L_MEMORY_CLASS_STORAGE_BUFFER) {
        const auto& base2 = *(const MemoryUniformBuffer*)base.get();
        return std::shared_ptr<Memory>(new MemoryStorageBuffer(ty, std::move(ac), base2.binding, base2.set));
      } else {
        panic("unsupported access chain base");
      }


    } else {
      panic("unsupported memory indirection");
    }

    return out;
  }
  std::shared_ptr<Memory> parse_mem(spv::Id id) {
    return parse_mem(lookup_instr(id));
  }

  std::shared_ptr<Expr> parse_expr(const InstructionRef& instr) {
    std::shared_ptr<Expr> out;
    spv::Op op = instr.op();
    if (op == spv::Op::OpConstant) {
      auto e = instr.extract_params();
      std::vector<uint32_t> lits;
      auto ty = parse_ty(instr.result_ty_id());
      while (e) {
        lits.emplace_back(e.read_u32());
      }
      out = std::shared_ptr<Expr>(new ExprConstant(ty, std::move(lits)));
    } else if (op == spv::Op::OpConstantTrue) {
      auto ty = parse_ty(instr.result_ty_id());
      std::vector<uint32_t> lits;
      lits.emplace_back(1);
      out = std::shared_ptr<Expr>(new ExprConstant(ty, std::move(lits)));
    } else if (op == spv::Op::OpConstantFalse) {
      auto ty = parse_ty(instr.result_ty_id());
      std::vector<uint32_t> lits;
      lits.emplace_back(0);
      out = std::shared_ptr<Expr>(new ExprConstant(ty, std::move(lits)));
    } else if (op == spv::Op::OpLoad) {
      auto ty = parse_ty(instr.result_ty_id());
      auto e = instr.extract_params();
      auto src_ptr = parse_mem(e.read_id());
      out = std::shared_ptr<Expr>(new ExprLoad(ty, src_ptr));
    } else if (op == spv::Op::OpIAdd || op == spv::Op::OpFAdd) {
      auto ty = parse_ty(instr.result_ty_id());
      auto e = instr.extract_params();
      auto a = parse_expr(e.read_id());
      auto b = parse_expr(e.read_id());
      out = std::shared_ptr<Expr>(new ExprAdd(ty, a, b));
    } else if (
      op == spv::Op::OpSLessThan ||
      op == spv::Op::OpULessThan ||
      op == spv::Op::OpFOrdLessThan
      ) {
      auto ty = parse_ty(instr.result_ty_id());
      auto e = instr.extract_params();
      auto a = parse_expr(e.read_id());
      auto b = parse_expr(e.read_id());
      out = std::shared_ptr<Expr>(new ExprLt(ty, a, b));
    } else if (
      op == spv::Op::OpConvertFToS ||
      op == spv::Op::OpConvertSToF ||
      op == spv::Op::OpConvertFToU ||
      op == spv::Op::OpConvertUToF
      ) {
      auto dst_ty = parse_ty(instr.result_ty_id());
      auto e = instr.extract_params();
      auto src = parse_expr(e.read_id());
      out = std::shared_ptr<Expr>(new ExprTypeCast(dst_ty, src));
    } else if (op == spv::Op::OpIEqual || op == spv::Op::OpFOrdEqual) {
      auto ty = parse_ty(instr.result_ty_id());
      auto e = instr.extract_params();
      auto a = parse_expr(e.read_id());
      auto b = parse_expr(e.read_id());
      out = std::shared_ptr<Expr>(new ExprEq(ty, a, b));
    }
    return out;
  }
  std::shared_ptr<Expr> parse_expr(spv::Id id) {
    return parse_expr(lookup_instr(id));
  }

  std::shared_ptr<Stmt> parse_stmt(const InstructionRef& instr) {
    std::shared_ptr<Stmt> out;
    spv::Op op = instr.op();
    if (op == spv::Op::OpStore) {
      auto e = instr.extract_params();
      auto dst_ptr = parse_mem(e.read_id());
      auto value = parse_expr(e.read_id());
      assert(dst_ptr != nullptr);
      assert(value != nullptr);
      out = (std::shared_ptr<Stmt>)(Stmt*)new StmtStore(dst_ptr, value);
    }
    return out;
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
      std::shared_ptr<Expr> cond_expr = parse_expr(cond);

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
      body_branch.cond = parse_expr(cond);
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
      std::shared_ptr<Stmt> stmt = parse_stmt(instr);
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
