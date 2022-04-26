#include "spv/ast.hpp"

using namespace liong;

struct SelectionMerge {
  spv::Id merge_target;
  spv::SelectionControlMask sel_ctrl;
  SelectionMerge(const InstructionRef& instr) {
    auto e = instr.extract_params();
    merge_target = e.read_id();
    sel_ctrl = e.read_u32_as<spv::SelectionControlMask>();
  }
};
struct LoopMerge {
  spv::Id merge_target;
  spv::Id continue_target;
  spv::LoopControlMask loop_ctrl;
  LoopMerge(const InstructionRef& instr) {
    auto e = instr.extract_params();
    merge_target = e.read_id();
    continue_target = e.read_id();
    loop_ctrl = e.read_u32_as<spv::LoopControlMask>();
  }
};
struct Branch {
  spv::Id target_label;
  Branch(const InstructionRef& instr) {
    auto e = instr.extract_params();
    target_label = e.read_id();
  }
};
struct BranchConditional {
  spv::Id cond;
  spv::Id then_label;
  spv::Id else_label;
  BranchConditional(const InstructionRef& instr) {
    auto e = instr.extract_params();
    cond = e.read_id();
    then_label = e.read_id();
    else_label = e.read_id();
  }
};

struct ParserState {
  InstructionRef cur;
  InstructionRef cur_block_label;

  std::shared_ptr<uint8_t> loop_handle;
  InstructionRef loop_continue_target;
  InstructionRef loop_merge_target;
  InstructionRef loop_back_edge_target;
  std::shared_ptr<uint8_t> sel_handle;
  InstructionRef sel_merge_target;

  bool is_inside_block = false;
  bool is_first_block = true;
};

struct ControlFlowParser {
  SpirvModule& mod;
  ParserState parser_state;

  std::vector<StmtRef> stmts;




  ControlFlowParser(
    SpirvModule& mod,
    ParserState&& parser_state
  ) : mod(mod), parser_state(std::forward<ParserState>(parser_state)), stmts() {}



  void parse_label() {
    const InstructionRef& instr = parser_state.cur;
    assert(instr.op() == spv::Op::OpLabel);

    if (instr == parser_state.sel_merge_target) {
      parser_state.cur = nullptr;
    } else if (instr == parser_state.loop_merge_target) {
      auto stmt = StmtRef(new StmtLoopMerge(parser_state.loop_handle));
      stmts.emplace_back(std::move(stmt));
      parser_state.cur = nullptr;
    } else if (instr == parser_state.loop_continue_target) {
      auto stmt = StmtRef(new StmtLoopContinue(parser_state.loop_handle));
      stmts.emplace_back(std::move(stmt));
      parser_state.cur = nullptr;
    } else if (instr == parser_state.loop_back_edge_target) {
      auto stmt = StmtRef(new StmtLoopBackEdge(parser_state.loop_handle));
      stmts.emplace_back(stmt);
      parser_state.cur = nullptr;
    } else {
      parser_state.cur_block_label = instr;
      parser_state.cur = instr.next();
      parser_state.is_inside_block = true;
    }
  }
  void parse_func_var() {
    const InstructionRef& instr = parser_state.cur;
    if (instr.op() != spv::Op::OpVariable) {
      parser_state.is_first_block = false;
      return;
    }

    auto ptr_ty = mod.ty_map.at(instr.result_ty_id());
    assert(ptr_ty->cls == L_TYPE_CLASS_POINTER);
    auto var_ty = ((const TypePointer*)ptr_ty.get())->inner;

    auto e = instr.extract_params();
    spv::StorageClass store_cls = e.read_u32_as<spv::StorageClass>();
    // Merely function vairables.
    assert(store_cls == spv::StorageClass::Function);
    auto mem = MemoryRef(new MemoryFunctionVariable(var_ty, {}, std::make_shared<uint8_t>()));
    mod.mem_map.emplace(instr, mem);

    parser_state.cur = instr.next();
  }
  bool parse_access_chain() {
    const InstructionRef& instr = parser_state.cur;
    if (instr.op() != spv::Op::OpAccessChain) { return false; }

    auto ptr_ty = mod.ty_map.at(instr.result_ty_id());
    assert(ptr_ty->cls == L_TYPE_CLASS_POINTER);
    auto ty = ((const TypePointer*)ptr_ty.get())->inner;

    auto e = instr.extract_params();
    auto base = mod.mem_map.at(e.read_id());
    auto ac = base->ac;
    while (e) {
      ac.emplace_back(mod.expr_map.at(e.read_id()));
    }

    MemoryRef mem = nullptr;
    switch (base->cls) {
    case L_MEMORY_CLASS_FUNCTION_VARIABLE:
    {
      const auto& base2 = *(const MemoryFunctionVariable*)base.get();
      mem = MemoryRef(new MemoryFunctionVariable(ty, std::move(ac), base2.handle));
      break;
    }
    case L_MEMORY_CLASS_UNIFORM_BUFFER:
    {
      const auto& base2 = *(const MemoryUniformBuffer*)base.get();
      mem = MemoryRef(new MemoryUniformBuffer(ty, std::move(ac), base2.binding, base2.set));
      break;
    }
    case L_MEMORY_CLASS_STORAGE_BUFFER:
    {
      const auto& base2 = *(const MemoryUniformBuffer*)base.get();
      mem = MemoryRef(new MemoryStorageBuffer(ty, std::move(ac), base2.binding, base2.set));
      break;
    }
    default: unimplemented();
    }
    mod.mem_map.emplace(instr, std::move(mem));

    parser_state.cur = instr.next();
    return true;
  }
  bool parse_func_expr() {
    const InstructionRef& instr = parser_state.cur;

    ExprRef expr;
    switch (instr.op()) {
    case spv::Op::OpLoad:
    {
      auto ty = mod.ty_map.at(instr.result_ty_id());
      auto e = instr.extract_params();
      auto src_ptr = mod.mem_map.at(e.read_id());
      expr = ExprRef(new ExprLoad(ty, src_ptr));
      break;
    }
    case spv::Op::OpIAdd:
    case spv::Op::OpFAdd:
    {
      auto ty = mod.ty_map.at(instr.result_ty_id());
      auto e = instr.extract_params();
      auto a = mod.expr_map.at(e.read_id());
      auto b = mod.expr_map.at(e.read_id());
      expr = ExprRef(new ExprAdd(ty, a, b));
      break;
    }
    case spv::Op::OpIMul:
    case spv::Op::OpFMul:
    {
      auto ty = mod.ty_map.at(instr.result_ty_id());
      auto e = instr.extract_params();
      auto a = mod.expr_map.at(e.read_id());
      auto b = mod.expr_map.at(e.read_id());
      expr = ExprRef(new ExprMul(ty, a, b));
      break;
    }
    case spv::Op::OpSDiv:
    case spv::Op::OpUDiv:
    case spv::Op::OpFDiv:
    {
      auto ty = mod.ty_map.at(instr.result_ty_id());
      auto e = instr.extract_params();
      auto a = mod.expr_map.at(e.read_id());
      auto b = mod.expr_map.at(e.read_id());
      expr = ExprRef(new ExprDiv(ty, a, b));
      break;
    }
    case spv::Op::OpSMod:
    case spv::Op::OpUMod:
    {
      auto ty = mod.ty_map.at(instr.result_ty_id());
      auto e = instr.extract_params();
      auto a = mod.expr_map.at(e.read_id());
      auto b = mod.expr_map.at(e.read_id());
      expr = ExprRef(new ExprMod(ty, a, b));
      break;
    }
    case spv::Op::OpSLessThan:
    case spv::Op::OpULessThan:
    case spv::Op::OpFOrdLessThan:
    {
      auto ty = mod.ty_map.at(instr.result_ty_id());
      auto e = instr.extract_params();
      auto a = mod.expr_map.at(e.read_id());
      auto b = mod.expr_map.at(e.read_id());
      expr = ExprRef(new ExprLt(ty, a, b));
      break;
    }
    case spv::Op::OpSGreaterThan:
    case spv::Op::OpUGreaterThan:
    case spv::Op::OpFOrdGreaterThan:
    {
      auto ty = mod.ty_map.at(instr.result_ty_id());
      auto e = instr.extract_params();
      auto a = mod.expr_map.at(e.read_id());
      auto b = mod.expr_map.at(e.read_id());
      expr = ExprRef(new ExprLt(ty, b, a));
      break;
    }
    case spv::Op::OpConvertFToS:
    case spv::Op::OpConvertSToF:
    case spv::Op::OpConvertFToU:
    case spv::Op::OpConvertUToF:
    {
      auto dst_ty = mod.ty_map.at(instr.result_ty_id());
      auto e = instr.extract_params();
      auto src = mod.expr_map.at(e.read_id());
      expr = ExprRef(new ExprTypeCast(dst_ty, src));
      break;
    }
    case spv::Op::OpIEqual:
    case spv::Op::OpFOrdEqual:
    {
      auto ty = mod.ty_map.at(instr.result_ty_id());
      auto e = instr.extract_params();
      auto a = mod.expr_map.at(e.read_id());
      auto b = mod.expr_map.at(e.read_id());
      expr = ExprRef(new ExprEq(ty, a, b));
      break;
    }
    case spv::Op::OpINotEqual:
    case spv::Op::OpFOrdNotEqual:
    {
      auto ty = mod.ty_map.at(instr.result_ty_id());
      auto e = instr.extract_params();
      auto a = mod.expr_map.at(e.read_id());
      auto b = mod.expr_map.at(e.read_id());
      expr = ExprRef(new ExprNot(ty, new ExprEq(ty, a, b)));
      break;
    }
    case spv::Op::OpSelect:
    {
      auto ty = mod.ty_map.at(instr.result_id());
      auto e = instr.extract_params();
      auto cond = mod.expr_map.at(e.read_id());
      assert(cond->ty->is<TypeBool>());
      auto a = mod.expr_map.at(e.read_id());
      auto b = mod.expr_map.at(e.read_id());
      expr = ExprRef(new ExprSelect(ty, cond, a, b));
    }
    default:
      return false;
    }
    mod.expr_map.emplace(instr, expr);

    parser_state.cur = instr.next();
    return true;
  }
  bool parse_func_ctrl_flow_merge_stmt() {
    const InstructionRef& instr = parser_state.cur;

    InstructionRef merge_target;
    switch (instr.op()) {
    case spv::Op::OpSelectionMerge:
    {
      SelectionMerge sr(instr);
      merge_target = mod.lookup_instr(sr.merge_target);
      std::shared_ptr<uint8_t> handle = std::make_shared<uint8_t>();

      ParserState parser_state2 = parser_state;
      parser_state2.cur = instr.next();
      parser_state2.sel_handle = handle;
      parser_state2.sel_merge_target = merge_target;
      auto body_stmt = parse(mod, std::move(parser_state2));

      auto stmt = body_stmt;
      stmts.emplace_back(std::move(stmt));
      break;
    }
    case spv::Op::OpLoopMerge:
    {
      LoopMerge sr(instr);
      merge_target = mod.lookup_instr(sr.merge_target);
      auto continue_target = mod.lookup_instr(sr.continue_target);
      std::shared_ptr<uint8_t> handle = std::make_shared<uint8_t>();

      ParserState body_parser_state2 = parser_state;
      body_parser_state2.cur = instr.next();
      body_parser_state2.loop_handle = handle;
      body_parser_state2.loop_merge_target = merge_target;
      body_parser_state2.loop_continue_target = continue_target;
      body_parser_state2.loop_back_edge_target = parser_state.cur_block_label;
      auto body_stmt = parse(mod, std::move(body_parser_state2));

      ParserState continue_parser_state2 = parser_state;
      continue_parser_state2.cur = continue_target.next();
      continue_parser_state2.loop_handle = handle;
      continue_parser_state2.loop_merge_target = merge_target;
      continue_parser_state2.loop_continue_target = continue_target;
      continue_parser_state2.loop_back_edge_target = parser_state.cur_block_label;
      auto continue_stmt = parse(mod, std::move(continue_parser_state2));

      auto stmt = StmtRef(new StmtLoop(body_stmt, continue_stmt, handle));
      stmts.emplace_back(std::move(stmt));
      break;
    }
    default:
      return false;
    }

    parser_state.cur = merge_target;
    parser_state.is_inside_block = false;
    return true;
  }
  bool parse_func_ctrl_flow_branch_stmt() {
    const InstructionRef& instr = parser_state.cur;

    StmtRef stmt;
    switch (instr.op()) {
    case spv::Op::OpBranch:
    {
      parser_state.is_inside_block = false;
      Branch sr(instr);
      parser_state.cur = mod.lookup_instr(sr.target_label);
      break;
    }
    case spv::Op::OpBranchConditional:
    {
      parser_state.is_inside_block = false;
      BranchConditional sr(instr);

      auto cond_expr = mod.expr_map.at(sr.cond);

      ParserState then_parser_state2 = parser_state;
      then_parser_state2.cur = mod.lookup_instr(sr.then_label);
      auto then_stmt = parse(mod, std::move(then_parser_state2));

      ParserState else_parser_state2 = parser_state;
      else_parser_state2.cur = mod.lookup_instr(sr.else_label);
      auto else_stmt = parse(mod, std::move(else_parser_state2));

      auto stmt = StmtRef(new StmtConditionalBranch(cond_expr, then_stmt, else_stmt));
      stmts.emplace_back(std::move(stmt));
      parser_state.cur = nullptr;
      break;
    }
    default: return false;
    }

    return true;
  }
  bool parse_func_ctrl_flow_tail_stmt() {
    const InstructionRef& instr = parser_state.cur;

    switch (instr.op()) {
    case spv::Op::OpReturn:
    {
      parser_state.is_inside_block = false;
      StmtRef stmt = new StmtReturn;
      stmts.emplace_back(std::move(stmt));
      break;
    }
    case spv::Op::OpUnreachable:
    {
      parser_state.is_inside_block = false;
      break;
    }
    default: return false;
    }

    parser_state.cur = nullptr;
    return true;
  }
  bool parse_func_behavior_stmt() {
    const InstructionRef& instr = parser_state.cur;

    StmtRef stmt;
    switch (instr.op()) {
    case spv::Op::OpStore:
    {
      auto e = instr.extract_params();
      auto dst_ptr = mod.mem_map.at(e.read_id());
      auto value = mod.expr_map.at(e.read_id());
      assert(dst_ptr != nullptr);
      assert(value != nullptr);
      stmt = StmtRef(new StmtStore(dst_ptr, value));
      break;
    }
    default: return false;
    }
    stmts.emplace_back(std::move(stmt));

    parser_state.cur = instr.next();
    return true;
  }




  // Returns true if the process can continue; otherwise false is returned as an
  // interruption.
  void parse_one() {
    if (!parser_state.is_inside_block) {
      parse_label();
    } else {
      if (parser_state.is_first_block) {
        // Parse function variables if it's the first block. Function variables
        // are always declared in the front of the first block (entry block) of a
        // function.
        parse_func_var();
      } else {
        bool succ = false;
        succ |= parse_access_chain();
        succ |= parse_func_expr();
        succ |= parse_func_ctrl_flow_tail_stmt();
        succ |= parse_func_ctrl_flow_merge_stmt();
        succ |= parse_func_ctrl_flow_branch_stmt();
        succ |= parse_func_behavior_stmt();
        assert(succ);
      }

    }
  }
  void parse() {
    while (parser_state.cur != nullptr) {
      parse_one();
    }
  }

  static StmtRef parse(
    SpirvModule& mod,
    ParserState&& parser_state
  ) {
    ControlFlowParser parser(mod, std::forward<ParserState>(parser_state));
    parser.parse();

    auto stmt = StmtRef(new StmtBlock(std::move(parser.stmts)));
    return stmt;
  }
  static StmtRef parse(
    SpirvModule& mod,
    InstructionRef cur
  ) {
    ParserState parser_state {};
    parser_state.cur = cur;
    return parse(mod, std::move(parser_state));
  }
};

std::map<std::string, StmtRef> extract_entry_points(SpirvModule& mod) {
  std::map<std::string, StmtRef> out {};

  for (const auto& pair : mod.entry_points) {
    const auto& entry_point = mod.funcs.at(pair.second.func);
    StmtRef root = ControlFlowParser::parse(mod, entry_point.entry_label);
    out.emplace(pair.second.name, std::move(root));
  }

  return out;
}

