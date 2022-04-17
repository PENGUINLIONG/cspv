// Type class registry.
// @PENGUINLIONG
#pragma once
#include <memory>
#include "gft/assert.hpp"
#include "spv/node.hpp"

enum TypeClass {
  L_TYPE_CLASS_VOID,
  L_TYPE_CLASS_BOOL,
  L_TYPE_CLASS_INT,
  L_TYPE_CLASS_FLOAT,
  L_TYPE_CLASS_STRUCT,
  L_TYPE_CLASS_POINTER,
};
struct Type : public Node {
  const TypeClass cls;

  virtual bool is_same_as(const Type& other) const {
    liong::unimplemented();
  }

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
  inline Type(TypeClass cls) : Node(L_NODE_VARIANT_TYPE), cls(cls) {}
};
