#include "gft/assert.hpp"
#include "gft/log.hpp"
#include "spv/mod.hpp"
#include "spv/ast.hpp"

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

  inline InstructionRef lookup_instr(spv::Id id) const {
    return out.abstr.id2instr_map.at(id);
  }

  inline uint32_t get_deco_u32(spv::Decoration deco, const InstructionRef& instr) const {
    return out.get_deco_u32(deco, instr);
  }
  inline const InstructionRef& get_deco_instr(spv::Decoration deco, const InstructionRef& instr) const {
    return out.get_deco_instr(deco, instr);
  }
  inline bool has_deco(spv::Decoration deco, const InstructionRef& instr) const {
    return out.has_deco(deco, instr);
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
      if (op == spv::Op::OpMemberDecorate) {
        auto e = instr.extract_params();
        InstructionRef target = lookup_instr(e.read_id());
        uint32_t imember = e.read_u32();
        spv::Decoration deco = e.read_u32_as<spv::Decoration>();
        MemberDecoration record {};
        record.deco = deco;
        record.imember = imember;
        record.instr = instr;
        out.instr2member_deco_map[target].emplace_back(std::move(record));
      } else {
        auto e = instr.extract_params();
        InstructionRef target = lookup_instr(e.read_id());
        spv::Decoration deco = e.read_u32_as<spv::Decoration>();
        Decoration record {};
        record.deco = deco;
        record.instr = instr;
        out.instr2deco_map[target].emplace_back(std::move(record));
      }

    }

    // Group and string decorations are unsupported.
    assert(!fetch_instr({
      spv::Op::OpDecorationGroup, spv::Op::OpGroupDecorate,
      spv::Op::OpGroupMemberDecorate, spv::Op::OpDecorateString,
      spv::Op::OpMemberDecorateString
      }), "group and string decorations are not supported");
  }


  std::shared_ptr<Type> parse_ty(const InstructionRef& instr) {
    switch (instr.op()) {
    case spv::Op::OpTypeVoid:
    {
      return std::shared_ptr<Type>(new TypeVoid);
    }
    case spv::Op::OpTypeBool:
    {
      return std::shared_ptr<Type>(new TypeBool);
    }
    case spv::Op::OpTypeInt:
    {
      auto e = instr.extract_params();
      uint32_t nbit = e.read_u32();
      bool is_signed = e.read_bool();
      return std::shared_ptr<Type>(new TypeInt(nbit, is_signed));
    }
    case spv::Op::OpTypeFloat:
    {
      auto e = instr.extract_params();
      uint32_t nbit = e.read_u32();
      return std::shared_ptr<Type>(new TypeFloat(nbit));
    }
    case spv::Op::OpTypeStruct:
    {
      auto e = instr.extract_params();
      std::vector<std::shared_ptr<Type>> members;
      while (e) {
        auto member_ty = out.ty_map.at(e.read_id());
        members.emplace_back(member_ty);
      }
      return std::shared_ptr<Type>(new TypeStruct(std::move(members)));
    }
    case spv::Op::OpTypePointer:
    {
      auto e = instr.extract_params();
      spv::StorageClass storage_cls = e.read_u32_as<spv::StorageClass>();
      auto inner = out.ty_map.at(e.read_id());
      return std::shared_ptr<Type>(new TypePointer(inner));
    }
    case spv::Op::OpTypeFunction:
    {
      return nullptr;
    }
    default: unimplemented();
    }
    return nullptr;
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
      auto id = instr.result_id();
      assert(id != L_INVALID_ID);
      auto ty = parse_ty(instr);
      out.ty_map.emplace(id, std::move(ty));
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

  std::shared_ptr<Expr> parse_const(const InstructionRef& instr) {
    switch (instr.op()) {
    case spv::Op::OpConstant:
    {
      auto e = instr.extract_params();
      std::vector<uint32_t> lits;
      auto ty = out.ty_map.at(instr.result_ty_id());
      while (e) {
        lits.emplace_back(e.read_u32());
      }
      return std::shared_ptr<Expr>(new ExprConstant(ty, std::move(lits)));
    }
    case spv::Op::OpConstantTrue:
    {
      auto ty = out.ty_map.at(instr.result_ty_id());
      std::vector<uint32_t> lits;
      lits.emplace_back(1);
      return std::shared_ptr<Expr>(new ExprConstant(ty, std::move(lits)));
    }
    case spv::Op::OpConstantFalse:
    {
      auto ty = out.ty_map.at(instr.result_ty_id());
      std::vector<uint32_t> lits;
      lits.emplace_back(0);
      return std::shared_ptr<Expr>(new ExprConstant(ty, std::move(lits)));
    }
    default: unimplemented();
    }
    return nullptr;
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
      spv::Id id = instr.result_id();
      assert(id != L_INVALID_ID);
      auto constant = parse_const(instr);
      out.expr_map.emplace(id, std::move(constant));
      return true;
    }

    return false;
  }

  std::shared_ptr<Memory> parse_global_mem(const InstructionRef& ptr) {
    auto op = ptr.op();
    if (op == spv::Op::OpVariable) {
      auto ptr_ty = out.ty_map.at(ptr.result_ty_id());
      assert(ptr_ty->cls == L_TYPE_CLASS_POINTER);
      auto var_ty = ((const TypePointer*)ptr_ty.get())->inner;

      auto e = ptr.extract_params();
      spv::StorageClass store_cls = e.read_u32_as<spv::StorageClass>();
      assert(store_cls != spv::StorageClass::Function,
        "function variables are parsed within functions");

      // Descriptor resources.
      if (has_deco(spv::Decoration::BufferBlock, ptr)) {
        store_cls = spv::StorageClass::StorageBuffer;
      }
      uint32_t binding = get_deco_u32(spv::Decoration::Binding, ptr);
      uint32_t set = get_deco_u32(spv::Decoration::DescriptorSet, ptr);
      if (store_cls == spv::StorageClass::Uniform) {
        return std::shared_ptr<Memory>(new MemoryUniformBuffer(var_ty, {}, binding, set));
      } else if (store_cls == spv::StorageClass::StorageBuffer) {
        return std::shared_ptr<Memory>(new MemoryStorageBuffer(var_ty, {}, binding, set));
      } else {
        panic("unsupported memory allocation");
      }

    } else {
      panic("unsupported memory indirection");
    }

    return nullptr;
  }

  bool visit_global_var_declrs() {
    InstructionRef instr {};
    if (instr = fetch_instr(spv::Op::OpVariable)) {
      spv::Id id = instr.result_id();
      assert(id != L_INVALID_ID);
      auto mem = parse_global_mem(instr);
      out.mem_map.emplace(id, std::move(mem));
      return true;
    }

    return false;
  }
  void visit_global_declrs() {
    // TODO: (penguinliong) Deal with non-semantic instructions.
    for (;;) {
      bool success =
        visit_ty_declrs() ||
        visit_const_declrs() ||
        visit_global_var_declrs();
      if (!success) { break; }
    }
  }



  void visit_func_params(SpirvFunction& func) {
    InstructionRef instr;
    while (instr = fetch_instr(spv::Op::OpFunctionParameter)) {
      // TODO: (penguinliong) Do something with parameters? Do we need that?
    }
  }
  void visit_func_block(SpirvFunction& func) {
    InstructionRef instr;
    if (instr = fetch_any_instr()) {
      if (instr.op() == spv::Op::OpLabel) {
        if (!func.entry_label) {
          func.entry_label = instr;
        }
        out.label_map.emplace(instr.result_id(), instr);
      }
    } else {
      panic("unexpected end of intruction stream");
    }
  }
  void visit_func_body(SpirvFunction& func) {
    InstructionRef instr;
    while (!ate()) {
      if (instr = fetch_instr(spv::Op::OpFunctionEnd)) { break; }
      visit_func_block(func);
    }
  }

  void visit_func() {
    InstructionRef instr {};
    if (instr = fetch_instr(spv::Op::OpFunction)) {
      auto e = instr.extract_params();

      SpirvFunction func {};
      func.func_ctrl = e.read_u32_as<spv::FunctionControlMask>();
      func.return_ty = lookup_instr(instr.result_ty_id());
      func.func_ty = lookup_instr(e.read_id());

      visit_func_params(func);
      visit_func_body(func);

      out.funcs.emplace(instr, std::move(func));
    }
  }
  void visit_funcs() {
    InstructionRef instr {};
    while (!ate()) {
      visit_func();
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
    visit_global_declrs();
    visit_funcs();
    assert(ate(), "spirv is not exhausted");
  }
};

SpirvModule parse_spirv_module(SpirvAbstract&& abstr) {
  SpirvVisitor visitor(std::forward<SpirvAbstract>(abstr));
  visitor.visit();
  return visitor.out;
}
