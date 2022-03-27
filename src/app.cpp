#include <string>
#include <vector>
#include <set>
#include <map>
#include "gft/log.hpp"
#include "gft/args.hpp"
#include "gft/util.hpp"
#include "gft/assert.hpp"
#define SPV_ENABLE_UTILITY_CODE
#include "spirv/unified1/spirv.hpp11"

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

const spv::Id L_INVALID_ID = spv::Id(0);

struct SpirvHeader {
  uint32_t magic;
  uint32_t version;
  uint32_t generator_magic;
  uint32_t bound;
  uint32_t reserved;
};
struct InstructionParameterExtractor {
  const uint32_t* cur;
  const uint32_t* end;

  inline InstructionParameterExtractor(
    const uint32_t* cur,
    const uint32_t* end
  ) : cur(cur), end(end) {}

  inline uint32_t read_u32() {
    assert(cur < end);
    return *(cur++);
  }
  inline spv::Id read_id() {
    return spv::Id(read_u32());
  }
  inline const char* read_str() {
    const char* out = (const char*)cur;
    size_t size = std::strlen((const char*)cur) + 1;
    cur += util::div_up(size, sizeof(uint32_t));
    return out;
  }
  template<typename T>
  inline T read_u32_as() {
    return T(read_u32());
  }
};
struct InstructionRef {
  const uint32_t* inner;

  inline InstructionRef() : inner(nullptr) {}
  inline InstructionRef(const uint32_t* inner) : inner(inner) {}
  inline InstructionRef(const InstructionRef& rhs) : inner(rhs.inner) {}
  inline InstructionRef(InstructionRef&& rhs) :
    inner(std::exchange(rhs.inner, nullptr)) {}

  inline InstructionRef& operator=(const InstructionRef& rhs) {
    inner = rhs.inner;
    return *this;
  }
  inline InstructionRef& operator=(InstructionRef&& rhs) {
    inner = std::exchange(rhs.inner, nullptr);
    return *this;
  }

  inline InstructionRef& operator++() {
    inner += len();
    return *this;
  }
  inline InstructionRef operator++(int) {
    InstructionRef out(inner);
    inner += len();
    return out;
  }

  constexpr bool operator==(std::nullptr_t) const { return inner == nullptr; }
  constexpr bool operator!=(std::nullptr_t) const { return inner != nullptr; }
  constexpr bool operator==(const InstructionRef& rhs) const {
    return inner == rhs.inner;
  }
  constexpr bool operator!=(const InstructionRef& rhs) const {
    return inner != rhs.inner;
  }
  constexpr bool operator<(const InstructionRef& rhs) const {
    return inner < rhs.inner;
  }
  constexpr bool operator>=(const InstructionRef& rhs) const {
    return inner >= rhs.inner;
  }

  constexpr operator bool() const {
    return *this != nullptr;
  }

  constexpr const uint32_t* words() const {
    return inner;
  }

  constexpr spv::Op op() const {
    return spv::Op(*inner & 0xFFFF);
  }
  constexpr size_t len() const {
    return *inner >> 16;
  }

  inline spv::Id result_ty_id() const {
    bool has_result_ty_id, has_result_id;
    spv::HasResultAndType(op(), &has_result_id, &has_result_ty_id);

    if (has_result_ty_id) {
      return inner[1];
    } else {
      return L_INVALID_ID;
    }
  }
  inline spv::Id result_id() const {
    bool has_result_ty_id, has_result_id;
    spv::HasResultAndType(op(), &has_result_id, &has_result_ty_id);

    if (has_result_id) {
      return has_result_ty_id ? inner[2] : inner[1];
    } else {
      return L_INVALID_ID;
    }
  }

  inline InstructionParameterExtractor extract_params() const {
    bool has_result_ty_id, has_result_id;
    spv::HasResultAndType(op(), &has_result_id, &has_result_ty_id);

    const uint32_t* param_beg;
    if (has_result_id) {
      param_beg = has_result_ty_id ? inner + 3 : inner + 2;
    } else {
      param_beg = inner + 1;
    }
    const uint32_t* param_end = inner + len();

    return InstructionParameterExtractor(param_beg, param_end);
  }
};
struct SpirvAbstract {
  SpirvHeader head;
  std::map<spv::Id, InstructionRef> id2instr_map;
  InstructionRef beg;
  InstructionRef end;
};

SpirvAbstract scan_spirv(const std::vector<uint32_t>& spv) {
  SpirvAbstract out {};
  out.head.magic = spv[0];
  out.head.version = spv[1];
  out.head.generator_magic = spv[2];
  out.head.bound = spv[3];
  out.head.reserved = spv[4];

  out.beg = spv.data() + 5;
  out.end = spv.data() + spv.size();
  InstructionRef cur = out.beg;
  while (cur < out.end) {
    const InstructionRef instr(cur);
    spv::Op op = instr.op();

    // Ignore source line debug info.
    if (op == spv::Op::OpLine || op == spv::Op::OpNoLine) {
      goto done;
    }

    spv::Id result_id = instr.result_id();
    if (result_id) {
      auto it = out.id2instr_map.find(result_id);
      if (it == out.id2instr_map.end()) {
        out.id2instr_map.emplace(std::make_pair(result_id, cur));
      } else {
        panic("result id #", result_id, " is assigned by more than one "
          "instructions");
      }
    }

  done:
    ++cur;
  }

  return out;
}



struct SpirvInstructionIterator {
  InstructionRef cur;
  const InstructionRef end;

  const InstructionRef& peek() const {
    return cur;
  }
  InstructionRef next() {
    return cur++;
  }
};


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

struct SpirvFunctionBlock {
  InstructionRef label;
  std::vector<InstructionRef> instrs;
  InstructionRef merge;
  InstructionRef term;
};
struct SpirvFunction {
  InstructionRef return_ty;
  spv::FunctionControlMask func_ctrl;
  InstructionRef func_ty;
  InstructionRef entry_block_label;
  std::map<InstructionRef, SpirvFunctionBlock> label_instr2blocks;
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
      return cur++;
    } else {
      return nullptr;
    }
  }
  inline InstructionRef fetch_instr(spv::Op expected_op) {
    if (!ate()) {
      InstructionRef out(cur);
      if (cur.op() == expected_op) {
        return cur++;
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
    while (
      visit_ty_declrs() || visit_const_declrs() || visit_global_var_declrs()
    ) {}
  }


  void visit_func_params() {
    InstructionRef instr;
    while (instr = fetch_instr(spv::Op::OpFunctionParameter)) {
      // TODO: (penguinliong) Do something with parameters? Do we need that?
    }
  }
  bool visit_func_block_term() {
    InstructionRef instr;
    assert(!ate());

    if (instr = fetch_instr(spv::Op::OpSelectionMerge)) {
    } else if (instr = fetch_instr(spv::Op::OpLoopMerge)) {
    }

    if (instr = fetch_instr(spv::Op::OpBranch)) {
      return true;
    } else if (instr = fetch_instr(spv::Op::OpBranchConditional)) {
      return true;
    } else if (instr = fetch_instr(spv::Op::OpSwitch)) {
      return true;
    } else if (instr = fetch_instr(spv::Op::OpReturn)) {
      return true;
    } else if (instr = fetch_instr(spv::Op::OpReturnValue)) {
      return true;
    }
      
    assert(!fetch_instr({
      spv::Op::OpKill, spv::Op::OpTerminateInvocation, spv::Op::OpUnreachable
    }), "unsupported termination instruction");
    return false;
  }
  void visit_func_stmt() {
    InstructionRef instr;
    assert(instr = fetch_any_instr(), "unexpected end of spirv");
  }
  void visit_func_block() {
    InstructionRef instr;
    assert(instr = fetch_instr(spv::Op::OpLabel),
      "function block label not found");

    while (!ate()) {
      if (visit_func_block_term()) { return; }
      visit_func_stmt();
    }
  }
  void visit_func_body(SpirvFunction& func) {
    InstructionRef instr;
    while (!ate()) {
      if (instr = fetch_instr(spv::Op::OpFunctionEnd)) { return; }
      visit_func_block();
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


void guarded_main() {
  if (CFG.in_file_path.empty()) {
    panic("source file path not given");
  }
  std::vector<uint32_t> spv = load_spv(CFG.in_file_path.c_str());
  SpirvAbstract abstr = scan_spirv(spv);
  SpirvVisitor visitor(abstr);
  visitor.visit();

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
