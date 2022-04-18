// Expression op implementations.
// @PENGUINLIONG
#pragma once
#include <memory>
#include "spv/expr-reg.hpp"
#include "spv/mem-reg.hpp"
#include "spv/ty-reg.hpp"

struct ExprConstant : public Expr {
  static const ExprOp OP = L_EXPR_OP_CONSTANT;
  const std::vector<uint32_t> lits;

  inline ExprConstant(
    const std::shared_ptr<Type>& ty,
    std::vector<uint32_t>&& lits
  ) : Expr(OP, ty), lits(lits) {}

  virtual bool is_constexpr() const override final {
    return true;
  }
};

struct ExprLoad : public Expr {
  static const ExprOp OP = L_EXPR_OP_LOAD;
  const std::shared_ptr<Memory> src_ptr;

  inline ExprLoad(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Memory>& src_ptr
  ) : Expr(OP, ty), src_ptr(src_ptr) {}

  virtual bool is_constexpr() const override final {
    return src_ptr->cls == L_MEMORY_CLASS_UNIFORM_BUFFER;
  }
};

struct ExprBinaryOp : public Expr {
  const std::shared_ptr<Expr> a;
  const std::shared_ptr<Expr> b;

  inline ExprBinaryOp(
    ExprOp op,
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : Expr(op, ty), a(a), b(b) {
    liong::assert(a && b && a->ty->is_same_as(*b->ty));
  }
  virtual bool is_constexpr() const override final {
    return a->is_constexpr() && b->is_constexpr();
  }
};

struct ExprAdd : public ExprBinaryOp {
  static const ExprOp OP = L_EXPR_OP_ADD;
  inline ExprAdd(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(OP, ty, a, b) {
    liong::assert(ty->is_same_as(*a->ty));
  }
};
struct ExprSub : public ExprBinaryOp {
  static const ExprOp OP = L_EXPR_OP_SUB;
  inline ExprSub(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(OP, ty, a, b) {
    liong::assert(ty->is_same_as(*a->ty));
  }
};

struct ExprLt : public ExprBinaryOp {
  static const ExprOp OP = L_EXPR_OP_LT;
  inline ExprLt(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(OP, ty, a, b) {
    liong::assert(ty->cls == L_TYPE_CLASS_BOOL);
  }
};
struct ExprEq : public ExprBinaryOp {
  static const ExprOp OP = L_EXPR_OP_EQ;
  inline ExprEq(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(OP, ty, a, b) {
    liong::assert(ty->cls == L_TYPE_CLASS_BOOL);
  }
};
struct ExprTypeCast : public Expr {
  static const ExprOp OP = L_EXPR_OP_TYPE_CAST;
  std::shared_ptr<Expr> src;
  inline ExprTypeCast(
    const std::shared_ptr<Type>& dst_ty,
    const std::shared_ptr<Expr>& src
  ) : Expr(OP, dst_ty), src(src) {}

  virtual bool is_constexpr() const override final {
    return src->is_constexpr();
  }
};
