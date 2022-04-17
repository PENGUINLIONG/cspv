// Type class implementations.
// @PENGUINLIONG
#pragma once
#include "spv/type-reg.hpp"

struct TypeVoid : public Type {
  TypeVoid() : Type(L_TYPE_CLASS_VOID) {}
  virtual bool is_same_as(const Type& other) const override final {
    return other.cls == cls;
  }
};
struct TypeBool : public Type {
  TypeBool() : Type(L_TYPE_CLASS_BOOL) {}
  virtual bool is_same_as(const Type& other) const override final {
    return other.cls == cls;
  }
};
struct TypeInt : public Type {
  uint32_t nbit;
  bool is_signed;
  TypeInt(uint32_t nbit, bool is_signed) : Type(L_TYPE_CLASS_INT),
    nbit(nbit), is_signed(is_signed) {}
  virtual bool is_same_as(const Type& other) const override final {
    if (other.cls != cls) { return false; }
    const auto& other2 = (const TypeInt&)other;
    return other2.nbit == nbit && other2.is_signed == is_signed;
  }
};
struct TypeFloat : public Type {
  uint32_t nbit;
  TypeFloat(uint32_t nbit) : Type(L_TYPE_CLASS_FLOAT), nbit(nbit) {}
  virtual bool is_same_as(const Type& other) const override final {
    if (other.cls != cls) { return false; }
    const auto& other2 = (const TypeFloat&)other;
    return other2.nbit == nbit;
  }
};
// For structs specifically, we assume that the memory layout for uniform
// buffers are in `std140` and storage buffers in `std430`. The layout depends
// on where the struct type is used.
struct TypeStruct : public Type {
  std::vector<std::shared_ptr<Type>> members;
  TypeStruct(
    std::vector<std::shared_ptr<Type>>&& members
  ) : Type(L_TYPE_CLASS_STRUCT),
    members(std::forward<std::vector<std::shared_ptr<Type>>>(members)) {}
  virtual bool is_same_as(const Type& other) const override final {
    if (other.cls != cls) { return false; }
    const auto& other2 = (const TypeStruct&)other;
    for (size_t i = 0; i < members.size(); ++i) {
      if (!members.at(i)->is_same_as(*other2.members.at(i))) {
        return false;
      }
    }
    return true;
  }
};
struct TypePointer : public Type {
  std::shared_ptr<Type> inner;
  TypePointer(const std::shared_ptr<Type>& inner) : Type(L_TYPE_CLASS_POINTER),
    inner(inner) {}
  virtual bool is_same_as(const Type& other) const override final {
    if (other.cls != cls) { return false; }
    const auto& other2 = (const TypePointer&)other;
    return is_same_as(*other2.inner);
  }
};
