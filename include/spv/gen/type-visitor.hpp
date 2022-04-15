// GENERATED SOURCE; DO NOT MODIFY.
// Type class visitor.
// @PENGUINLIONG
#pragma once
#include "spv/type.hpp"

struct TypeVisitor {
  void visit_ty(const Type& ty) {
    switch (ty.cls) {
    case L_TYPE_CLASS_VOID: ty.visit(this); visit_ty(*(const TypeVoid*)&ty); break;
    case L_TYPE_CLASS_BOOL: ty.visit(this); visit_ty(*(const TypeBool*)&ty); break;
    case L_TYPE_CLASS_INT: ty.visit(this); visit_ty(*(const TypeInt*)&ty); break;
    case L_TYPE_CLASS_FLOAT: ty.visit(this); visit_ty(*(const TypeFloat*)&ty); break;
    case L_TYPE_CLASS_STRUCT: ty.visit(this); visit_ty(*(const TypeStruct*)&ty); break;
    case L_TYPE_CLASS_POINTER: ty.visit(this); visit_ty(*(const TypePointer*)&ty); break;
    default: liong::unreachable();
    }
  }
  virtual void visit_ty(const TypeVoid&) {}
  virtual void visit_ty(const TypeBool&) {}
  virtual void visit_ty(const TypeInt&) {}
  virtual void visit_ty(const TypeFloat&) {}
  virtual void visit_ty(const TypeStruct&) {}
  virtual void visit_ty(const TypePointer&) {}
};
