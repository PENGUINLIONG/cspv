// Type class implementations.
// @PENGUINLIONG
#pragma once
#include "spv/type-reg.hpp"

struct TypeVoid : public Type {
  static const TypeClass CLS = L_TYPE_CLASS_VOID;
  TypeVoid() : Type(CLS) {}
  virtual bool is_same_as(const Type& other) const override final {
    return other.cls == cls;
  }
};
struct TypeBool : public Type {
  static const TypeClass CLS = L_TYPE_CLASS_BOOL;
  TypeBool() : Type(CLS) {}
  virtual bool is_same_as(const Type& other) const override final {
    return other.cls == cls;
  }
};
struct TypeInt : public Type {
  static const TypeClass CLS = L_TYPE_CLASS_INT;
  uint32_t nbit;
  bool is_signed;
  TypeInt(uint32_t nbit, bool is_signed) : Type(CLS), nbit(nbit),
    is_signed(is_signed) {}
  virtual bool is_same_as(const Type& other) const override final {
    if (other.cls != cls) { return false; }
    const auto& other2 = (const TypeInt&)other;
    return other2.nbit == nbit && other2.is_signed == is_signed;
  }
};
struct TypeFloat : public Type {
  static const TypeClass CLS = L_TYPE_CLASS_FLOAT;
  uint32_t nbit;
  TypeFloat(uint32_t nbit) : Type(CLS), nbit(nbit) {}
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
  static const TypeClass CLS = L_TYPE_CLASS_STRUCT;
  std::vector<std::shared_ptr<Type>> members;
  TypeStruct(
    std::vector<std::shared_ptr<Type>>&& members
  ) : Type(CLS),
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
  static const TypeClass CLS = L_TYPE_CLASS_POINTER;
  std::shared_ptr<Type> inner;
  TypePointer(const std::shared_ptr<Type>& inner) : Type(CLS), inner(inner) {}
  virtual bool is_same_as(const Type& other) const override final {
    if (other.cls != cls) { return false; }
    const auto& other2 = (const TypePointer&)other;
    return is_same_as(*other2.inner);
  }
};
