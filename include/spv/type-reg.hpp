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

protected:
  inline Type(TypeClass cls) : Node(L_NODE_VARIANT_TYPE), cls(cls) {}
};
