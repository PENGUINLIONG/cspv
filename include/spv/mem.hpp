// Memory class implementations.
// @PENGUINLIONG
#pragma once
#include "mem-reg.hpp"

struct MemoryFunctionVariable : public Memory {
  static const MemoryClass CLS = L_MEMORY_CLASS_FUNCTION_VARIABLE;
  const void* handle;

  inline MemoryFunctionVariable(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    const void* handle // Used to identify same-source local variable accesses.
  ) : Memory(CLS, ty, std::forward<AccessChain>(ac)), handle(handle) {}
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
  static const MemoryClass CLS = L_MEMORY_CLASS_UNIFORM_BUFFER;
  inline MemoryUniformBuffer(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : MemoryDescriptor(CLS, ty, std::forward<AccessChain>(ac), binding, set) {}
};
struct MemoryStorageBuffer : public MemoryDescriptor {
  static const MemoryClass CLS = L_MEMORY_CLASS_STORAGE_BUFFER;
  size_t offset;
  inline MemoryStorageBuffer(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : MemoryDescriptor(CLS, ty, std::forward<AccessChain>(ac), binding, set) {}
};
struct MemorySampledImage : public MemoryDescriptor {
  static const MemoryClass CLS = L_MEMORY_CLASS_SAMPLED_IMAGE;
  inline MemorySampledImage(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : MemoryDescriptor(CLS, ty, std::forward<AccessChain>(ac), binding, set) {}
};
struct MemoryStorageImage : public MemoryDescriptor {
  static const MemoryClass CLS = L_MEMORY_CLASS_STORAGE_IMAGE;
  inline MemoryStorageImage(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : MemoryDescriptor(CLS, ty, std::forward<AccessChain>(ac), binding, set) {}
};
