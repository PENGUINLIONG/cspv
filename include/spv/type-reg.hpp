// Type class registry.
// @PENGUINLIONG
#pragma once
#include "spv/dbg.hpp"

enum TypeClass {
  L_TYPE_CLASS_VOID,
  L_TYPE_CLASS_BOOL,
  L_TYPE_CLASS_INT,
  L_TYPE_CLASS_FLOAT,
  L_TYPE_CLASS_STRUCT,
  L_TYPE_CLASS_POINTER,
};
struct Type {
  const TypeClass cls;

  virtual void visit(struct TypeVisitor*) const {}

  virtual bool is_same_as(const Type& other) const {
    liong::unimplemented();
  }

  virtual void dbg_print(Debug& s) const { s << "type?"; }

protected:
  inline Type(TypeClass cls) : cls(cls) {}
};

inline Debug& operator<<(Debug& s, const Type& x) {
  x.dbg_print(s);
  return s;
}
