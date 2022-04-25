// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.
// Expr node implementation.
// @PENGUINLIONG
#pragma once
#include "node/reg.hpp"

typedef Reference<struct ExprPatternCapture> ExprPatternCaptureRef;
typedef Reference<struct ExprPatternBinaryOp> ExprPatternBinaryOpRef;
typedef Reference<struct ExprConstant> ExprConstantRef;
typedef Reference<struct ExprLoad> ExprLoadRef;
typedef Reference<struct ExprAdd> ExprAddRef;
typedef Reference<struct ExprSub> ExprSubRef;
typedef Reference<struct ExprLt> ExprLtRef;
typedef Reference<struct ExprEq> ExprEqRef;
typedef Reference<struct ExprNot> ExprNotRef;
typedef Reference<struct ExprTypeCast> ExprTypeCastRef;
typedef Reference<struct ExprSelect> ExprSelectRef;

struct ExprPatternCapture : public Expr {
  static const ExprOp OP = L_EXPR_OP_PATTERN_CAPTURE;
  ExprRef captured;

  inline ExprPatternCapture(
    const TypeRef& ty,
    const ExprRef& captured
  ) : Expr(L_EXPR_OP_PATTERN_CAPTURE, ty), captured(captured) {
    liong::assert(captured != nullptr);
  }
  inline ExprPatternCapture(const TypeRef& ty) : Expr(L_EXPR_OP_PATTERN_CAPTURE, ty) {}

  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(ty);
    drain->push(captured);
  }
};

struct ExprPatternBinaryOp : public Expr {
  static const ExprOp OP = L_EXPR_OP_PATTERN_BINARY_OP;
  std::shared_ptr<ExprOp> op;
  ExprRef a;
  ExprRef b;

  inline ExprPatternBinaryOp(
    const TypeRef& ty,
    std::shared_ptr<ExprOp> op,
    const ExprRef& a,
    const ExprRef& b
  ) : Expr(L_EXPR_OP_PATTERN_BINARY_OP, ty), op(op), a(a), b(b) {
    liong::assert(a != nullptr);
    liong::assert(b != nullptr);
  }
  inline ExprPatternBinaryOp(const TypeRef& ty) : Expr(L_EXPR_OP_PATTERN_BINARY_OP, ty) {}

  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(ty);
    drain->push(a);
    drain->push(b);
  }
};

struct ExprConstant : public Expr {
  static const ExprOp OP = L_EXPR_OP_CONSTANT;
  std::vector<uint32_t> lits;

  inline ExprConstant(
    const TypeRef& ty,
    std::vector<uint32_t> lits
  ) : Expr(L_EXPR_OP_CONSTANT, ty), lits(lits) {
  }

  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(ty);
  }
};

struct ExprLoad : public Expr {
  static const ExprOp OP = L_EXPR_OP_LOAD;
  MemoryRef src_ptr;

  inline ExprLoad(
    const TypeRef& ty,
    const MemoryRef& src_ptr
  ) : Expr(L_EXPR_OP_LOAD, ty), src_ptr(src_ptr) {
    liong::assert(src_ptr != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(ty);
    drain->push(src_ptr);
  }
};

struct ExprAdd : public Expr {
  static const ExprOp OP = L_EXPR_OP_ADD;
  ExprRef a;
  ExprRef b;

  inline ExprAdd(
    const TypeRef& ty,
    const ExprRef& a,
    const ExprRef& b
  ) : Expr(L_EXPR_OP_ADD, ty), a(a), b(b) {
    liong::assert(a != nullptr);
    liong::assert(b != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(ty);
    drain->push(a);
    drain->push(b);
  }
};

struct ExprSub : public Expr {
  static const ExprOp OP = L_EXPR_OP_SUB;
  ExprRef a;
  ExprRef b;

  inline ExprSub(
    const TypeRef& ty,
    const ExprRef& a,
    const ExprRef& b
  ) : Expr(L_EXPR_OP_SUB, ty), a(a), b(b) {
    liong::assert(a != nullptr);
    liong::assert(b != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(ty);
    drain->push(a);
    drain->push(b);
  }
};

struct ExprLt : public Expr {
  static const ExprOp OP = L_EXPR_OP_LT;
  ExprRef a;
  ExprRef b;

  inline ExprLt(
    const TypeRef& ty,
    const ExprRef& a,
    const ExprRef& b
  ) : Expr(L_EXPR_OP_LT, ty), a(a), b(b) {
    liong::assert(a != nullptr);
    liong::assert(b != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(ty);
    drain->push(a);
    drain->push(b);
  }
};

struct ExprEq : public Expr {
  static const ExprOp OP = L_EXPR_OP_EQ;
  ExprRef a;
  ExprRef b;

  inline ExprEq(
    const TypeRef& ty,
    const ExprRef& a,
    const ExprRef& b
  ) : Expr(L_EXPR_OP_EQ, ty), a(a), b(b) {
    liong::assert(a != nullptr);
    liong::assert(b != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(ty);
    drain->push(a);
    drain->push(b);
  }
};

struct ExprNot : public Expr {
  static const ExprOp OP = L_EXPR_OP_NOT;
  ExprRef a;

  inline ExprNot(
    const TypeRef& ty,
    const ExprRef& a
  ) : Expr(L_EXPR_OP_NOT, ty), a(a) {
    liong::assert(a != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(ty);
    drain->push(a);
  }
};

struct ExprTypeCast : public Expr {
  static const ExprOp OP = L_EXPR_OP_TYPE_CAST;
  ExprRef src;

  inline ExprTypeCast(
    const TypeRef& ty,
    const ExprRef& src
  ) : Expr(L_EXPR_OP_TYPE_CAST, ty), src(src) {
    liong::assert(src != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(ty);
    drain->push(src);
  }
};

struct ExprSelect : public Expr {
  static const ExprOp OP = L_EXPR_OP_SELECT;
  ExprRef cond;
  ExprRef a;
  ExprRef b;

  inline ExprSelect(
    const TypeRef& ty,
    const ExprRef& cond,
    const ExprRef& a,
    const ExprRef& b
  ) : Expr(L_EXPR_OP_SELECT, ty), cond(cond), a(a), b(b) {
    liong::assert(cond != nullptr);
    liong::assert(a != nullptr);
    liong::assert(b != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) const override final {
    drain->push(ty);
    drain->push(cond);
    drain->push(a);
    drain->push(b);
  }
};

constexpr bool is_expr_binary_op(ExprOp op) {
  switch (op) {
  case L_EXPR_OP_LT:
  case L_EXPR_OP_SUB:
  case L_EXPR_OP_ADD:
  case L_EXPR_OP_EQ:
    return true;
  default: return false;
  }
}
constexpr bool is_expr_unary_op(ExprOp op) {
  switch (op) {
  case L_EXPR_OP_NOT:
  case L_EXPR_OP_TYPE_CAST:
    return true;
  default: return false;
  }
}