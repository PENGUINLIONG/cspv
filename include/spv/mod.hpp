// Structured parsing or SPIR-V module.
// @PENGUINLIONG
#pragma once
#include "spv/abstr.hpp"

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
  SpirvAbstract abstr;

  std::map<InstructionRef, std::vector<InstructionRef>> instr2deco_map;

  std::vector<spv::Capability> caps;
  std::vector<std::string> exts;
  std::map<InstructionRef, std::string> ext_instrs;

  spv::AddressingModel addr_model;
  spv::MemoryModel mem_model;

  std::map<InstructionRef, SpirvEntryPoint> entry_points;

  std::map<InstructionRef, SpirvVariable> vars;
  std::map<InstructionRef, SpirvFunction> funcs;

  SpirvModule(SpirvAbstract&& abstr) : abstr(std::forward<SpirvAbstract>(abstr)) {}
};

SpirvModule parse_spirv_module(SpirvAbstract&& abstr);
