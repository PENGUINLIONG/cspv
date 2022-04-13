#include <string>
#include <vector>
#include <set>
#include <map>
#include "gft/log.hpp"
#include "gft/args.hpp"
#include "gft/util.hpp"
#include "gft/assert.hpp"
#include "spv/abstr.hpp"
#include "spv/instr.hpp"
#include "ast.hpp"

using namespace liong;



static const char* APP_NAME = "GraphiT-Template";
static const char* APP_DESC = "GraphiT project template.";


struct AppConfig {
  std::string in_file_path;
  bool verbose = false;
} CFG;

void initialize(int argc, const char** argv) {
  args::init_arg_parse(APP_NAME, APP_DESC);
  args::reg_arg<args::SwitchParser>("-v", "--verbose", CFG.verbose,
    "Produce extra amount of logs for debugging.");
  args::reg_arg<args::StringParser>("-i", "--in-file", CFG.in_file_path,
    "Path to source code.");
  args::parse_args(argc, argv);

  extern void log_cb(log::LogLevel lv, const std::string& msg);
  log::set_log_callback(log_cb);
  log::LogLevel level = CFG.verbose ?
    log::LogLevel::L_LOG_LEVEL_DEBUG : log::LogLevel::L_LOG_LEVEL_INFO; 
  log::set_log_filter_level(level);
}

std::vector<uint32_t> load_spv(const char* path) {
  std::vector<uint8_t> raw = util::load_file(path);

  assert(raw.size() % sizeof(uint32_t) == 0,
    "spirv corrupted: size is not aligned to 4");

  uint32_t magic = *(const uint32_t*)raw.data();
  assert(magic == spv::MagicNumber);

  const size_t n = raw.size() / sizeof(uint32_t);
  const uint32_t* beg = (const uint32_t*)raw.data();
  const uint32_t* end = beg + n;
  std::vector<uint32_t> spv(beg, end);
  return spv;
}



struct SpirvEntryPointExecutionModeCompute {
  uint32_t local_size_x;
  uint32_t local_size_y;
  uint32_t local_size_z;
  spv::Id local_size_x_id;
  spv::Id local_size_y_id;
  spv::Id local_size_z_id;
};
struct SpirvEntryPoint {
  spv::ExecutionModel exec_model;
  InstructionRef func;
  std::string name;
  std::vector<InstructionRef> interfaces;

  SpirvEntryPointExecutionModeCompute exec_mode_comp;
};
struct SpirvVariable {
  spv::StorageClass storage_class;
  InstructionRef ty;
  InstructionRef init_value;
};

struct Block {
  InstructionRef label;
  std::vector<InstructionRef> instrs;
  InstructionRef merge;
  InstructionRef term;
};
struct SpirvFunction {
  InstructionRef return_ty;
  spv::FunctionControlMask func_ctrl;
  InstructionRef func_ty;
  InstructionRef entry_label;
  InstructionRef return_label;
  std::map<InstructionRef, Block> blocks;
  std::map<InstructionRef, InstructionRef> doms;
};
struct SpirvModule {
  std::map<InstructionRef, std::vector<InstructionRef>> instr2deco_map;

  std::vector<spv::Capability> caps;
  std::vector<std::string> exts;
  std::map<InstructionRef, std::string> ext_instrs;

  spv::AddressingModel addr_model;
  spv::MemoryModel mem_model;

  std::map<InstructionRef, SpirvEntryPoint> entry_points;

  std::map<InstructionRef, SpirvVariable> vars;
  std::map<InstructionRef, SpirvFunction> funcs;
};

struct SpirvVisitor {
  const SpirvAbstract& abstr;
  InstructionRef cur;

  SpirvModule out;

  SpirvVisitor(const SpirvAbstract& abstr) :
    abstr(abstr), cur(abstr.beg), out() {}

  constexpr bool ate() const {
    return cur >= abstr.end;
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

  inline InstructionRef lookup_instr_id(spv::Id id) {
    return abstr.id2instr_map.at(id);
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
      entry_point.func = lookup_instr_id(e.read_id());
      const char* name = e.read_str();
      entry_point.name = name;
      while (e.cur < e.end) {
        InstructionRef interface = lookup_instr_id(e.read_id());
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

      SpirvEntryPoint& exec_point = out.entry_points.at(lookup_instr_id(id));
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
      InstructionRef target = lookup_instr_id(e.read_id());
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
      var.ty = lookup_instr_id(instr.result_ty_id());
      if (e.cur < e.end) {
        var.init_value = lookup_instr_id(e.read_id());
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
        func.return_ty = lookup_instr_id(instr.result_ty_id());
        func.func_ty = lookup_instr_id(e.read_id());

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

SpirvModule parse_spirv_module(const SpirvAbstract& abstr) {
  SpirvVisitor visitor(abstr);
  visitor.visit();
  return visitor.out;
}



struct ControlFlow;

enum BranchType {
  L_BRANCH_TYPE_NEVER,
  L_BRANCH_TYPE_CONDITION_THEN,
  L_BRANCH_TYPE_CONDITION_ELSE,
  L_BRANCH_TYPE_ALWAYS,
};
struct Branch {
  BranchType branch_ty;
  std::shared_ptr<Expr> cond;
  std::unique_ptr<ControlFlow> ctrl_flow;
};

struct ControlFlowSelection {
  std::vector<Branch> branches;
};
struct ControlFlowInitLoop {
  std::unique_ptr<ControlFlow> body;
};
struct ControlFlowLoop {
  std::shared_ptr<Expr> cond;
  std::unique_ptr<ControlFlow> body;
};
struct ControlFlowReturn {
  InstructionRef rv;
};
struct ControlFlow {
  InstructionRef label;
  std::unique_ptr<ControlFlow> next;
  std::vector<std::shared_ptr<Stmt>> stmts;
  // Extra parameters.
  std::unique_ptr<ControlFlowSelection> sel;
  std::unique_ptr<ControlFlowReturn> ret;
  std::unique_ptr<ControlFlowLoop> loop;
};

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
  const SpirvAbstract& abstr;
  const SpirvFunction& func;
  std::vector<MergeState> merge_states;

  ControlFlowGraphParser(const SpirvAbstract& abstr, const SpirvFunction& func)
    : abstr(abstr), func(func), merge_states()
  {
    merge_states.reserve(1); // Has at most 1 element.
  }

  inline const InstructionRef& fetch_instr(spv::Id id) const {
    return abstr.id2instr_map.at(id);
  }

  void push_merge_state(
    const InstructionRef& head_label,
    const InstructionRef& merge_instr
  ) {
    if (merge_instr.op() == spv::Op::OpSelectionMerge) {
      auto e = merge_instr.extract_params();
      const InstructionRef& merge_target_label = fetch_instr(e.read_id());
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
      const InstructionRef& merge_target_label = fetch_instr(e.read_id());
      const InstructionRef& continue_target_label = fetch_instr(e.read_id());
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

    assert(merge_states.size() == 1);
  }
  void pop_merge_state() {
    assert(merge_states.size() == 1);
    merge_states.pop_back();
  }

  std::unique_ptr<ControlFlow> parse_branch_conditional(const Block& block) {
    assert(!merge_states.empty());
    MergeState& merge_state = merge_states.back();

    auto e = block.term.extract_params();
    const InstructionRef& cond = fetch_instr(e.read_id());
    const InstructionRef& then_target_label = fetch_instr(e.read_id());
    const InstructionRef& else_target_label = fetch_instr(e.read_id());

    ControlFlow out {};
    out.label = block.label;
    out.next = parse(merge_state.merge_target_label);
    for (const InstructionRef& instr : block.instrs) {
      std::shared_ptr<Stmt> stmt = parse_stmt(abstr, instr);
      if (stmt != nullptr) {
        out.stmts.emplace_back(std::move(stmt));
      }
    }
    if (merge_state.sel) {
      std::shared_ptr<Expr> cond_expr = parse_expr(abstr, cond);

      Branch then_branch {};
      then_branch.branch_ty = L_BRANCH_TYPE_CONDITION_THEN;
      then_branch.cond = cond_expr;
      then_branch.ctrl_flow = parse(then_target_label);

      Branch else_branch {};
      else_branch.branch_ty = L_BRANCH_TYPE_CONDITION_ELSE;
      else_branch.cond = cond_expr;
      else_branch.ctrl_flow = parse(else_target_label);

      ControlFlowSelection sel {};
      sel.branches.emplace_back(std::move(then_branch));
      sel.branches.emplace_back(std::move(else_branch));
      out.sel = std::make_unique<ControlFlowSelection>(std::move(sel));
    } else if (merge_state.loop) {
      assert(else_target_label == merge_state.merge_target_label);

      ControlFlowLoop loop {};
      loop.cond = parse_expr(abstr, cond);
      loop.body = parse(then_target_label);
      out.loop = std::make_unique<ControlFlowLoop>(std::move(loop));
    }
    pop_merge_state();
    return std::make_unique<ControlFlow>(std::move(out));
  }
  std::unique_ptr<ControlFlow> parse_branch(const Block& block) {
    auto e = block.term.extract_params();

    ControlFlow out {};
    out.label = block.label;
    out.next = parse(fetch_instr(e.read_id()));
    for (const InstructionRef& instr : block.instrs) {
      std::shared_ptr<Stmt> stmt = parse_stmt(abstr, instr);
      if (stmt != nullptr) {
        out.stmts.emplace_back(std::move(stmt));
      }
    }
    return std::make_unique<ControlFlow>(std::move(out));
  }
  std::unique_ptr<ControlFlow> parse_return(const Block& block) {
    ControlFlow out {};
    out.label = block.label;
    for (const InstructionRef& instr : block.instrs) {
      std::shared_ptr<Stmt> stmt = parse_stmt(abstr, instr);
      if (stmt != nullptr) {
        out.stmts.emplace_back(std::move(stmt));
      }
    }
    return std::make_unique<ControlFlow>(std::move(out));
  }

  std::unique_ptr<ControlFlow> parse(const InstructionRef& label) {
    if (label == nullptr) { return nullptr; }

    const Block& block = func.blocks.at(label);
    if (!merge_states.empty()) {
      MergeState& merge_state = merge_states.back();
      // Limit recursion not to search out of the merge scope.
      if (label == merge_state.merge_target_label) {
        return nullptr;
      }
      // Break loops.
      if (merge_state.loop && merge_state.head_label == label) {
        return nullptr;
      }
    }

    if (block.merge != nullptr) {
      push_merge_state(label, block.merge);
    }

    switch (block.term.op()) {
    case spv::Op::OpBranchConditional: return parse_branch_conditional(block);
    case spv::Op::OpBranch: return parse_branch(block);
    case spv::Op::OpReturn: return parse_return(block);
    default: unreachable(); return nullptr;
    }

    assert(merge_states.empty());
  }
};

void guarded_main() {
  if (CFG.in_file_path.empty()) {
    panic("source file path not given");
  }
  std::vector<uint32_t> spv = load_spv(CFG.in_file_path.c_str());
  SpirvAbstract abstr = scan_spirv(spv);
  SpirvModule mod = parse_spirv_module(abstr);

  const auto& entry_point = mod.funcs.begin()->second;
  std::unique_ptr<ControlFlow> ctrl_flow =
    ControlFlowGraphParser(abstr, entry_point).parse(entry_point.entry_label);

   log::info("success");
}



// -----------------------------------------------------------------------------
// Usually you don't need to change things below.

int main(int argc, const char** argv) {
  initialize(argc, argv);
  //try {
    guarded_main();
  //} catch (const std::exception& e) {
  //  liong::log::error("application threw an exception");
  //  liong::log::error(e.what());
  //  liong::log::error("application cannot continue");
  //} catch (...) {
  //  liong::log::error("application threw an illiterate exception");
  //}

  return 0;
}

void log_cb(log::LogLevel lv, const std::string& msg) {
  using log::LogLevel;
  switch (lv) {
  case LogLevel::L_LOG_LEVEL_DEBUG:
    printf("[\x1b[90mDEBUG\x1B[0m] %s\n", msg.c_str());
    break;
  case LogLevel::L_LOG_LEVEL_INFO:
    printf("[\x1B[32mINFO\x1B[0m] %s\n", msg.c_str());
    break;
  case LogLevel::L_LOG_LEVEL_WARNING:
    printf("[\x1B[33mWARN\x1B[0m] %s\n", msg.c_str());
    break;
  case LogLevel::L_LOG_LEVEL_ERROR:
    printf("[\x1B[31mERROR\x1B[0m] %s\n", msg.c_str());
    break;
  }
}
