// Low-level types.
// @PENGUINLIONG
#pragma once
#include <memory>
#include <vector>
#include "counter.hpp"

namespace lo {

typedef std::shared_ptr<struct Type> TypeRef;
typedef uint32_t TypeClass;

struct Type {
  const TypeClass cls;

  inline Type(TypeClass cls) : cls(cls) {}

  template<typename T>
  inline as() { return static_cast<T&>(this); }
  template<typename T>
  inline as() const { return static_cast<const T&>(this); }
};
template<typename T>
struct Type_ : public Type {
  static const TypeClass CLS = Counter<Type>::get_id();
  Type_() : Type(CLS) {}
};

struct TypeVoid : public Type_<TypeVoid> {
};
struct TypeInt : public Type_<TypeInt> {
  bool is_signed;
  uint32_t nbit;
};
struct TypeFloat : public Type_<TypeFloat> {
  uint32_t nbit;
};
struct TypeArray : public Type_<TypeArray> {
  TypeRef elem_ty;
  uint64_t nelem;
};
struct TypeStruct : public Type_<TypeStruct> {
  std::vector<TypeRef> members;
};
struct TypePointer : public Type_<TypePointer> {
  TypeRef inner_ty;
};

} // namespace lo
