#include "spv/ast.hpp"

using namespace liong;

std::shared_ptr<Memory> parse_mem(const SpirvModule& mod, const InstructionRef& ptr) {
  std::shared_ptr<Memory> out;

  auto op = ptr.op();
  if (op == spv::Op::OpVariable) {
    auto ptr_ty = parse_ty(mod, ptr.result_ty_id());
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
    auto ptr_ty = parse_ty(mod, ptr.result_ty_id());
    assert(ptr_ty->cls == L_TYPE_CLASS_POINTER);
    auto ty = ((const TypePointer*)ptr_ty.get())->inner;

    auto e = ptr.extract_params();
    auto base = parse_mem(mod, e.read_id());
    AccessChain ac = base->ac;
    while (e) {
      ac.idxs.emplace_back(parse_expr(mod, e.read_id()));
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
std::shared_ptr<Memory> parse_mem(const SpirvModule& mod, spv::Id id) {
  return parse_mem(mod, mod.lookup_instr(id));
}
