// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.
// Type node implementation.
// @PENGUINLIONG
#pragma once
#include "node/reg.hpp"

typedef Reference<struct TypePatternCapture> TypePatternCaptureRef;
typedef Reference<struct TypeVoid> TypeVoidRef;
typedef Reference<struct TypeBool> TypeBoolRef;
typedef Reference<struct TypeInt> TypeIntRef;
typedef Reference<struct TypeFloat> TypeFloatRef;
typedef Reference<struct TypeStruct> TypeStructRef;
typedef Reference<struct TypePointer> TypePointerRef;

struct TypePatternCapture : public Type {
  static const TypeClass CLS = L_TYPE_CLASS_PATTERN_CAPTURE;
  TypeRef captured;

  inline TypePatternCapture(
    const TypeRef& captured
  ) : Type(L_TYPE_CLASS_PATTERN_CAPTURE), captured(captured) {
    liong::assert(captured != nullptr);
  }
  inline TypePatternCapture() : Type(L_TYPE_CLASS_PATTERN_CAPTURE) {}

  virtual bool structured_eq(TypeRef b_) const override final {
    if (!b_->is<TypePatternCapture>()) { return false; }
    const auto& b2_ = b_->as<TypePatternCapture>();
    if (!captured->structured_eq(b2_.captured)) { return false; }
    return true;
  }
  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(captured);
  }
};

struct TypeVoid : public Type {
  static const TypeClass CLS = L_TYPE_CLASS_VOID;

  inline TypeVoid(
  ) : Type(L_TYPE_CLASS_VOID) {
  }

  virtual bool structured_eq(TypeRef b_) const override final {
    if (!b_->is<TypeVoid>()) { return false; }
    const auto& b2_ = b_->as<TypeVoid>();
    return true;
  }
  virtual void collect_children(NodeDrain* drain) const override final {
  }
};

struct TypeBool : public Type {
  static const TypeClass CLS = L_TYPE_CLASS_BOOL;

  inline TypeBool(
  ) : Type(L_TYPE_CLASS_BOOL) {
  }

  virtual bool structured_eq(TypeRef b_) const override final {
    if (!b_->is<TypeBool>()) { return false; }
    const auto& b2_ = b_->as<TypeBool>();
    return true;
  }
  virtual void collect_children(NodeDrain* drain) const override final {
  }
};

struct TypeInt : public Type {
  static const TypeClass CLS = L_TYPE_CLASS_INT;
  uint32_t nbit;
  bool is_signed;

  inline TypeInt(
    uint32_t nbit,
    bool is_signed
  ) : Type(L_TYPE_CLASS_INT), nbit(nbit), is_signed(is_signed) {
  }

  virtual bool structured_eq(TypeRef b_) const override final {
    if (!b_->is<TypeInt>()) { return false; }
    const auto& b2_ = b_->as<TypeInt>();
    if (nbit != b2_.nbit) { return false; }
    if (is_signed != b2_.is_signed) { return false; }
    return true;
  }
  virtual void collect_children(NodeDrain* drain) const override final {
  }
};

struct TypeFloat : public Type {
  static const TypeClass CLS = L_TYPE_CLASS_FLOAT;
  uint32_t nbit;

  inline TypeFloat(
    uint32_t nbit
  ) : Type(L_TYPE_CLASS_FLOAT), nbit(nbit) {
  }

  virtual bool structured_eq(TypeRef b_) const override final {
    if (!b_->is<TypeFloat>()) { return false; }
    const auto& b2_ = b_->as<TypeFloat>();
    if (nbit != b2_.nbit) { return false; }
    return true;
  }
  virtual void collect_children(NodeDrain* drain) const override final {
  }
};

struct TypeStruct : public Type {
  static const TypeClass CLS = L_TYPE_CLASS_STRUCT;
  std::vector<TypeRef> members;

  inline TypeStruct(
    const std::vector<TypeRef>& members
  ) : Type(L_TYPE_CLASS_STRUCT), members(members) {
    for (const auto& x : members) { liong::assert(x != nullptr); }
  }

  virtual bool structured_eq(TypeRef b_) const override final {
    if (!b_->is<TypeStruct>()) { return false; }
    const auto& b2_ = b_->as<TypeStruct>();
    if (members.size() != b2_.members.size()) { return false; }
    for (size_t i = 0; i < members.size(); ++i) {
      if (!members.at(i)->structured_eq(b2_.members.at(i))) { return false; }
    }
    return true;
  }
  virtual void collect_children(NodeDrain* drain) const override final {
    for (const auto& x : members) { drain->push(x); }
  }
};

struct TypePointer : public Type {
  static const TypeClass CLS = L_TYPE_CLASS_POINTER;
  TypeRef inner;
  spv::StorageClass storage_cls;

  inline TypePointer(
    const TypeRef& inner,
    spv::StorageClass storage_cls
  ) : Type(L_TYPE_CLASS_POINTER), inner(inner), storage_cls(storage_cls) {
    liong::assert(inner != nullptr);
  }

  virtual bool structured_eq(TypeRef b_) const override final {
    if (!b_->is<TypePointer>()) { return false; }
    const auto& b2_ = b_->as<TypePointer>();
    if (!inner->structured_eq(b2_.inner)) { return false; }
    if (storage_cls != b2_.storage_cls) { return false; }
    return true;
  }
  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(inner);
  }
};
