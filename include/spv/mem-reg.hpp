// Memory class registry.
// @PENGUINLIONG
#pragma once
#include "spv/expr-reg.hpp"

struct AccessChain {
  std::vector<std::shared_ptr<Expr>> idxs;
};

enum MemoryClass {
  L_MEMORY_CLASS_FUNCTION_VARIABLE,
  L_MEMORY_CLASS_UNIFORM_BUFFER,
  L_MEMORY_CLASS_STORAGE_BUFFER,
  L_MEMORY_CLASS_SAMPLED_IMAGE,
  L_MEMORY_CLASS_STORAGE_IMAGE,
};
struct Memory : public Node {
  const MemoryClass cls;
  const std::shared_ptr<Type> ty;
  const AccessChain ac;

  template<typename T>
  const T& as() const {
    liong::assert(is<T>(), "memory class mismatched");
    return *(const T*)this;
  }
  template<typename T>
  bool is() const {
    return cls == T::CLS;
  }

protected:
  Memory(
    MemoryClass cls,
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac
  ) : Node(L_NODE_VARIANT_MEMORY), cls(cls), ty(ty),
    ac(std::forward<AccessChain>(ac)) {}
};
