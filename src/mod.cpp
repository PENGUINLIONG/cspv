#include "gft/assert.hpp"
#include "gft/log.hpp"
#include "spv/mod.hpp"

using namespace liong;

struct SpirvVisitor {
  SpirvModule out;
  InstructionRef cur;

  SpirvVisitor(SpirvAbstract&& abstr) :
    out(std::forward<SpirvAbstract>(abstr)), cur(abstr.beg) {}

  constexpr bool ate() const {
    return cur.inner >= out.abstr.end;
  }

  inline InstructionRef fetch_any_instr() {
    if (!ate()) {
      InstructionRef out = cur;
      cur = cur.next();
      return out;
    } else {
      return nullptr;
    }
  }
  inline InstructionRef fetch_instr(spv::Op expected_op) {
    if (!ate()) {
      InstructionRef out(cur);
      if (cur.op() == expected_op) {
        InstructionRef out = cur;
        cur = cur.next();
        return out;
      } else {
        return nullptr;
      }
    } else {
      return nullptr;
    }
  }
  inline InstructionRef fetch_instr(
    std::initializer_list<spv::Op> expected_ops
  ) {
    InstructionRef out;
    for (spv::Op expected_op : expected_ops) {
      if (out = fetch_instr(expected_op)) {
        break;
      }
    }
    return out;
  }

  inline InstructionRef lookup_instr(spv::Id id) {
    return out.abstr.id2instr_map.at(id);
  }

  void visit_caps() {
    InstructionRef instr;
    while (instr = fetch_instr(spv::Op::OpCapability)) {
      auto e = instr.extract_params();
      spv::Capability cap = e.read_u32_as<spv::Capability>();

      log::debug("required capability ", (uint32_t)cap);
      out.caps.emplace_back(cap);
    }
  }
  void visit_exts() {
    InstructionRef instr;
    while (instr = fetch_instr(spv::Op::OpExtension)) {
      auto e = instr.extract_params();
      const char* ext = e.read_str();

      log::debug("required extension ", ext);
      out.exts.emplace_back(ext);
    }
  }
  void visit_ext_instrs() {
    InstructionRef instr;
    while (instr = fetch_instr(spv::Op::OpExtInstImport)) {
      auto e = instr.extract_params();
      const char* ext_instr_name = e.read_str();

      log::debug("imported extension instruction '", ext_instr_name, "'");
      out.ext_instrs[instr] = ext_instr_name;
    }
  }
  void visit_mem_model() {
    InstructionRef instr = fetch_instr(spv::Op::OpMemoryModel);
    assert(instr, "memory model instruction missing");

    auto e = instr.extract_params();
    out.addr_model = e.read_u32_as<spv::AddressingModel>();
    out.mem_model = e.read_u32_as<spv::MemoryModel>();
  }
  void visit_entry_points() {
    InstructionRef instr;
    while (instr = fetch_instr(spv::Op::OpEntryPoint)) {
      auto e = instr.extract_params();

      SpirvEntryPoint entry_point {};
      entry_point.exec_model = e.read_u32_as<spv::ExecutionModel>();
      entry_point.func = lookup_instr(e.read_id());
      const char* name = e.read_str();
      entry_point.name = name;
      while (e.cur < e.end) {
        InstructionRef interface = lookup_instr(e.read_id());
        entry_point.interfaces.emplace_back(interface);
      }
      auto old = out.entry_points.emplace(
        std::make_pair(entry_point.func, std::move(entry_point)));
      assert(old.second, "entry point '", name, "' is already declared");
    }
  }
  void visit_exec_modes() {
    InstructionRef instr;
    while (instr = fetch_instr({
      spv::Op::OpExecutionMode, spv::Op::OpExecutionModeId
      })) {
      auto e = instr.extract_params();

      spv::Id id = e.read_id();
      spv::ExecutionMode exec_mode = e.read_u32_as<spv::ExecutionMode>();

      SpirvEntryPoint& exec_point = out.entry_points.at(lookup_instr(id));
      if (exec_mode == spv::ExecutionMode::LocalSize) {
        assert(exec_point.exec_model == spv::ExecutionModel::GLCompute);
        exec_point.exec_mode_comp.local_size_x = e.read_u32();
        exec_point.exec_mode_comp.local_size_y = e.read_u32();
        exec_point.exec_mode_comp.local_size_z = e.read_u32();
      } else if (exec_mode == spv::ExecutionMode::LocalSize) {
        assert(exec_point.exec_model == spv::ExecutionModel::GLCompute);
        exec_point.exec_mode_comp.local_size_x_id = e.read_id();
        exec_point.exec_mode_comp.local_size_y_id = e.read_id();
        exec_point.exec_mode_comp.local_size_z_id = e.read_id();
      } else {
        assert("unsupported execution model ", (uint32_t)exec_mode);
      }
    }
  }
  void visit_debug_instrs() {
    InstructionRef instr;
    while (instr = fetch_instr({
      spv::Op::OpString, spv::Op::OpSourceExtension, spv::Op::OpSource,
      spv::Op::OpSourceContinued, spv::Op::OpName, spv::Op::OpMemberName,
      spv::Op::OpModuleProcessed
      })) {
      spv::Op op = instr.op();
      // TODO: (penguinliong) Not necessarily processing these.
    }
  }
  void visit_annotations() {
    InstructionRef instr;
    while (instr = fetch_instr({
      spv::Op::OpDecorate, spv::Op::OpMemberDecorate, spv::Op::OpDecorateId
      })) {
      spv::Op op = instr.op();

      // Allowed decoration instructions.
      auto e = instr.extract_params();
      InstructionRef target = lookup_instr(e.read_id());
      out.instr2deco_map[target].emplace_back(instr);
    }

    // Group and string decorations are unsupported.
    assert(!fetch_instr({
      spv::Op::OpDecorationGroup, spv::Op::OpGroupDecorate,
      spv::Op::OpGroupMemberDecorate, spv::Op::OpDecorateString,
      spv::Op::OpMemberDecorateString
      }), "group and string decorations are not supported");
  }


  // Type, constants, global variable declaration.
  bool visit_ty_declrs() {
    InstructionRef instr;
    if (instr = fetch_instr({
      spv::Op::OpTypeVoid, spv::Op::OpTypeBool, spv::Op::OpTypeInt,
      spv::Op::OpTypeFloat, spv::Op::OpTypeVector, spv::Op::OpTypeMatrix,
      spv::Op::OpTypeImage, spv::Op::OpTypeSampler, spv::Op::OpTypeSampledImage,
      spv::Op::OpTypeArray, spv::Op::OpTypeRuntimeArray, spv::Op::OpTypeStruct,
      spv::Op::OpTypePointer, spv::Op::OpTypeFunction
      })) {
      // TODO: (penguinliong) Do we need typing details atm?
      return true;
    }

    // These types are not supported.
    assert(!fetch_instr({
      spv::Op::OpTypeOpaque, spv::Op::OpTypeEvent, spv::Op::OpTypeDeviceEvent,
      spv::Op::OpTypeReserveId, spv::Op::OpTypeQueue, spv::Op::OpTypePipe,
      spv::Op::OpTypeForwardPointer, spv::Op::OpTypePipeStorage,
      spv::Op::OpTypeNamedBarrier
      }));
    return false;
  }
  bool visit_const_declrs() {
    InstructionRef instr;
    if (instr = fetch_instr({
      spv::Op::OpConstantTrue, spv::Op::OpConstantFalse, spv::Op::OpConstant,
      spv::Op::OpConstantComposite, spv::Op::OpConstantSampler,
      spv::Op::OpConstantNull, spv::Op::OpSpecConstantTrue,
      spv::Op::OpSpecConstantFalse, spv::Op::OpSpecConstant,
      spv::Op::OpSpecConstantComposite, spv::Op::OpSpecConstantOp
      })) {
      return true;
    }

    return false;
  }
  bool visit_global_var_declrs() {
    InstructionRef instr {};
    if (instr = fetch_instr(spv::Op::OpVariable)) {
      SpirvVariable var;
      auto e = instr.extract_params();

      var.storage_class = e.read_u32_as<spv::StorageClass>();
      var.ty = lookup_instr(instr.result_ty_id());
      if (e.cur < e.end) {
        var.init_value = lookup_instr(e.read_id());
      }

      assert(var.storage_class != spv::StorageClass::Function,
        "function variable cannot be declared at a global scope");
      out.vars.emplace(std::make_pair(instr, std::move(var)));
      return true;
    }

    return false;
  }
  void visit_declrs() {
    // TODO: (penguinliong) Deal with non-semantic instructions.
    for (;;) {
      bool success = false;
      success |= visit_ty_declrs();
      success |= visit_const_declrs();
      success |= visit_global_var_declrs();
      if (!success) { break; }
    }
  }


  void visit_func_params() {
    InstructionRef instr;
    while (instr = fetch_instr(spv::Op::OpFunctionParameter)) {
      // TODO: (penguinliong) Do something with parameters? Do we need that?
    }
  }
  bool visit_func_merge_term(Block& block) {
    InstructionRef instr;
    assert(!ate());

    if (instr = fetch_instr({
      spv::Op::OpLoopMerge, spv::Op::OpSelectionMerge
      })) {
      assert(!block.merge, "already found a merge instruction");
      block.merge = instr;
      return true;
    }

    return false;
  }
  bool visit_func_block_term(Block& block) {
    InstructionRef instr;
    assert(!ate());
    assert(!block.term, "already found a termination instruction");

    if (instr = fetch_instr({
      spv::Op::OpBranch, spv::Op::OpBranchConditional, spv::Op::OpSwitch,
      spv::Op::OpReturn, spv::Op::OpReturnValue
      })) {
      block.term = instr;
      return true;
    }

    assert(!fetch_instr({
      spv::Op::OpKill, spv::Op::OpTerminateInvocation, spv::Op::OpUnreachable
      }), "unsupported termination instruction");
    return false;
  }
  void visit_func_stmt(Block& block) {
    InstructionRef instr;
    assert(instr = fetch_any_instr(), "unexpected end of spirv");
    block.instrs.emplace_back(instr);
  }
  Block visit_func_block(SpirvFunction& func) {
    InstructionRef instr;
    if (instr = fetch_instr(spv::Op::OpLabel)) {
      if (!func.entry_label) {
        func.entry_label = instr;
      }
    } else {
      panic("function block label not found");
    }

    Block block {};
    block.label = instr;

    while (!ate()) {
      if (visit_func_merge_term(block)) { continue; }
      if (visit_func_block_term(block)) { break; }
      visit_func_stmt(block);
    }

    {
      spv::Op term_op = block.term.op();
      if (term_op == spv::Op::OpReturn || term_op == spv::Op::OpReturnValue) {
        assert(func.return_label == nullptr,
          "a function with multiple returns is unsupported");
        func.return_label = instr;
      }
    }

    return block;
  }
  void visit_func_body(SpirvFunction& func) {
    InstructionRef instr;
    while (!ate()) {
      if (instr = fetch_instr(spv::Op::OpFunctionEnd)) { break; }
      Block block = visit_func_block(func);
      InstructionRef label = block.label;
      func.blocks.emplace(std::make_pair(label, std::move(block)));
    }
  }
  void visit_funcs() {
    InstructionRef instr {};
    while (!ate()) {
      SpirvFunction func {};

      if (instr = fetch_instr(spv::Op::OpFunction)) {
        auto e = instr.extract_params();
        func.func_ctrl = e.read_u32_as<spv::FunctionControlMask>();
        func.return_ty = lookup_instr(instr.result_ty_id());
        func.func_ty = lookup_instr(e.read_id());

        visit_func_params();
        visit_func_body(func);

        out.funcs.emplace(std::make_pair(instr, std::move(func)));
      } else {
        panic("unexpected instruction");
      }

    }
  }


  void visit() {
    visit_caps();
    visit_exts();
    visit_ext_instrs();
    visit_mem_model();
    visit_entry_points();
    visit_exec_modes();
    visit_debug_instrs();
    visit_annotations();
    visit_declrs();
    visit_funcs();
    assert(ate(), "spirv is not exhausted");
  }
};

SpirvModule parse_spirv_module(SpirvAbstract&& abstr) {
  SpirvVisitor visitor(std::forward<SpirvAbstract>(abstr));
  visitor.visit();
  return visitor.out;
}
