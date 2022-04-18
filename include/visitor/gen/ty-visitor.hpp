// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.
// Type visitor.
// @PENGUINLIONG
#pragma once
#include "spv/ty.hpp"

struct TypeVisitor {
  virtual void visit_ty(const Type& ty) {
    switch (ty.cls) {
    case L_TYPE_CLASS_VOID: visit_ty_(*(const TypeVoid*)&ty); break;
    case L_TYPE_CLASS_BOOL: visit_ty_(*(const TypeBool*)&ty); break;
    case L_TYPE_CLASS_INT: visit_ty_(*(const TypeInt*)&ty); break;
    case L_TYPE_CLASS_FLOAT: visit_ty_(*(const TypeFloat*)&ty); break;
    case L_TYPE_CLASS_STRUCT: visit_ty_(*(const TypeStruct*)&ty); break;
    case L_TYPE_CLASS_POINTER: visit_ty_(*(const TypePointer*)&ty); break;
    default: liong::unreachable();
    }
  }
  virtual void visit_ty_(const TypeVoid&);
  virtual void visit_ty_(const TypeBool&);
  virtual void visit_ty_(const TypeInt&);
  virtual void visit_ty_(const TypeFloat&);
  virtual void visit_ty_(const TypeStruct&);
  virtual void visit_ty_(const TypePointer&);
};

template<typename TType>
struct TypeFunctorVisitor : public TypeVisitor {
  std::function<void(const TType&)> f;
  TypeFunctorVisitor(std::function<void(const TType&)>&& f) :
    f(std::forward<std::function<void(const TType&)>>(f)) {}

  virtual void visit_ty_(const TType& ty) override final {
    f(ty);
  }
};
template<typename TType>
void visit_ty_functor(
  std::function<void(const TType&)>&& f,
  const Type& x
) {
  TypeFunctorVisitor<TType> visitor(
    std::forward<std::function<void(const TType&)>>(f));
  visitor.visit_ty(x);
}