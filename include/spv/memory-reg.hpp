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
struct Memory {
  const MemoryClass cls;
  const std::shared_ptr<Type> ty;
  const AccessChain ac;

  virtual void dbg_print(Debug& s) const { s << "memory?"; }

protected:
  Memory(
    MemoryClass cls,
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac
  ) : cls(cls), ty(ty), ac(std::forward<AccessChain>(ac)) {}
};

inline Debug& operator<<(Debug& s, const AccessChain& x) {
  bool first = true;
  for (const auto& idx : x.idxs) {
    if (first) {
      first = false;
    } else {
      s << ",";
    }
    s << *idx;
  }
  return s;
}
inline Debug& operator<<(Debug& s, const Memory& x) {
  x.dbg_print(s);
  return s;
}
