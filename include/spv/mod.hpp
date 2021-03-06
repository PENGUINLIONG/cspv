// Semantical segmentation of the order-dependent executable portion of SPIR-V
// module.
// @PENGUINLIONG
#pragma once
#include "spv/abstr.hpp"
#include "node/gen/ty.hpp"
#include "node/gen/mem.hpp"
#include "node/gen/expr.hpp"
#include "node/gen/stmt.hpp"

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

struct SpirvFunction {
  InstructionRef return_ty;
  spv::FunctionControlMask func_ctrl;
  InstructionRef func_ty;
  InstructionRef entry_label;
};
struct Decoration {
  spv::Decoration deco;
  InstructionRef instr;
};
struct MemberDecoration {
  spv::Decoration deco;
  uint32_t imember;
  InstructionRef instr;
};
struct SpirvModule {
  SpirvAbstract abstr;

  std::map<InstructionRef, std::vector<Decoration>> instr2deco_map;
  std::map<InstructionRef, std::vector<MemberDecoration>> instr2member_deco_map;

  std::vector<spv::Capability> caps;
  std::vector<std::string> exts;
  std::map<InstructionRef, std::string> ext_instrs;

  spv::AddressingModel addr_model;
  spv::MemoryModel mem_model;

  std::map<InstructionRef, SpirvEntryPoint> entry_points;
  std::map<InstructionRef, SpirvFunction> funcs;

  // Things that have IDs and cannot be forward referenced.
  std::map<spv::Id, TypeRef> ty_map;
  std::map<spv::Id, MemoryRef> mem_map;
  std::map<spv::Id, ExprRef> expr_map;
  std::map<spv::Id, InstructionRef> label_map;

  inline SpirvModule(SpirvAbstract&& abstr) :
    abstr(std::forward<SpirvAbstract>(abstr)) {}

  inline InstructionRef get_deco_instr(
    spv::Decoration deco,
    const InstructionRef& instr
  ) const {
    for (const auto& x : instr2deco_map.at(instr)) {
      if (x.deco == deco) { return x.instr; }
    }
    return nullptr;
  }
  inline InstructionRef get_member_deco_instr(
    spv::Decoration deco,
    uint32_t imember,
    const InstructionRef& instr
  ) const {
    for (const auto& x : instr2member_deco_map.at(instr)) {
      if (x.imember == imember && x.deco == deco) { return x.instr; }
    }
    return nullptr;
  }
  inline bool has_deco(
    spv::Decoration deco,
    const InstructionRef& instr
  ) const {
    return get_deco_instr(deco, instr) != nullptr;
  }
  inline uint32_t get_deco_u32(
    spv::Decoration deco,
    const InstructionRef& instr
  ) const {
    InstructionRef deco_instr = get_deco_instr(deco, instr);
    liong::assert(deco_instr);
    auto e = deco_instr.extract_params();
    e.read_id();
    e.read_u32();
    return e.read_u32();
  }

  inline const InstructionRef& lookup_instr(spv::Id id) const {
    return abstr.id2instr_map.at(id);
  }
};

SpirvModule parse_spirv_module(SpirvAbstract&& abstr);
