// Memory class implementations.
// @PENGUINLIONG
#pragma once
#include "memory-reg.hpp"

struct MemoryFunctionVariable : public Memory {
  const void* handle;

  inline MemoryFunctionVariable(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    const void* handle // Used to identify same-source local variable accesses.
  ) : Memory(L_MEMORY_CLASS_FUNCTION_VARIABLE, ty,
    std::forward<AccessChain>(ac)), handle(handle) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "$" << s.get_var_name_by_handle(handle) << ":" << *ty;
  }
};

struct MemoryDescriptor : public Memory {
  uint32_t binding;
  uint32_t set;

protected:
  inline MemoryDescriptor(
    MemoryClass cls,
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : Memory(cls, ty, std::forward<AccessChain>(ac)),
    binding(binding), set(set) {}
};

struct MemoryUniformBuffer : public MemoryDescriptor {
  inline MemoryUniformBuffer(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : MemoryDescriptor(L_MEMORY_CLASS_UNIFORM_BUFFER, ty,
    std::forward<AccessChain>(ac), binding, set) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "UniformBuffer@" << binding << "," << set << "[" << ac << "]:" << *ty;
  }
};
struct MemoryStorageBuffer : public MemoryDescriptor {
  size_t offset;
  inline MemoryStorageBuffer(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : MemoryDescriptor(L_MEMORY_CLASS_STORAGE_BUFFER, ty,
    std::forward<AccessChain>(ac), binding, set) {}
};
struct MemorySampledImage : public MemoryDescriptor {
  inline MemorySampledImage(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : MemoryDescriptor(L_MEMORY_CLASS_SAMPLED_IMAGE, ty,
    std::forward<AccessChain>(ac), binding, set) {}
};
struct MemorStorageImage : public MemoryDescriptor {
  inline MemorStorageImage(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : MemoryDescriptor(L_MEMORY_CLASS_STORAGE_IMAGE, ty,
    std::forward<AccessChain>(ac), binding, set) {}
};

std::shared_ptr<Memory> parse_mem(const SpirvModule& mod, const InstructionRef& ptr);
std::shared_ptr<Memory> parse_mem(const SpirvModule& mod, spv::Id id);
