// SSA memory handle.
// @PENGUINLIONG
#pragma once
#include "lo/type.hpp"

namespace lo {

typedef std::shared_ptr<struct Memory> MemoryRef;
typedef uint32_t MemoryClass;

struct Memory {
  const MemoryClass cls;

  inline Memory(MemoryClass cls) : cls(cls) {}

  template<typename T>
  inline as() { return static_cast<T&>(this); }
  template<typename T>
  inline as() const { return static_cast<const T&>(this); }
};
template<typename T>
struct Memory_ : public Memory {
  static const MemoryClass CLS = Counter<Memory>::get_id();
  Memory_() : Memory(CLS) {}
};

struct MemoryPrivate : public Memory_<MemoryPrivate> {
  TypeRef ty;
};
struct MemoryUniformBuffer : public Memory_<MemoryUniformBuffer> {
  TypeRef ty;
  uint32_t desc;
  uint32_t bind;
};
struct MemoryStorageBuffer : public Memory_<MemoryStorageBuffer> {
  TypeRef ty;
  uint32_t desc;
  uint32_t bind;
};

} // namespace lo
