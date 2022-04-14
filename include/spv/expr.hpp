// Expression op implementations.
// @PENGUINLIONG
#pragma once
#include "spv/expr-reg.hpp"
#include "spv/type-reg.hpp"

struct ExprConstant : public Expr {
  const std::vector<uint32_t> lits;

  inline ExprConstant(
    const std::shared_ptr<Type>& ty,
    std::vector<uint32_t>&& lits
  ) : Expr(L_EXPR_OP_CONSTANT, ty), lits(lits) {}

  virtual void dbg_print(Debug& s) const override final {
    if (ty->cls == L_TYPE_CLASS_INT) {
      const auto& ty2 = *(const TypeInt*)ty.get();
      if (ty2.is_signed) {
        if (ty2.nbit == 32) {
          s << *(const int32_t*)&lits[0];
        } else {
          liong::unimplemented();
        }
      } else {
        if (ty2.nbit == 32) {
          s << *(const uint32_t*)&lits[0];
        } else {
          liong::unimplemented();
        }
      }
    } else if (ty->cls == L_TYPE_CLASS_FLOAT) {
      const auto& ty2 = *(const TypeFloat*)ty.get();
      if (ty2.nbit == 32) {
        s << *(const float*)&lits[0];
      } else {
        liong::unimplemented();
      }
    } else if (ty->cls == L_TYPE_CLASS_BOOL) {
      s << (lits[0] != 0 ? "true" : "false");
    }
    s << ":" << *ty;
  }
};

struct ExprLoad : public Expr {
  const std::shared_ptr<Memory> src_ptr;

  inline ExprLoad(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Memory>& src_ptr
  ) : Expr(L_EXPR_OP_LOAD, ty), src_ptr(src_ptr) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "Load(" << *src_ptr << ")";
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
};

struct ExprAdd : public ExprBinaryOp {
  inline ExprAdd(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(L_EXPR_OP_ADD, ty, a, b) {
    liong::assert(ty->is_same_as(*a->ty));
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "(" << *a << " + " << *b << ")";
  }
};
struct ExprSub : public ExprBinaryOp {
  inline ExprSub(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(L_EXPR_OP_SUB, ty, a, b) {
    liong::assert(ty->is_same_as(*a->ty));
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "(" << *a << " - " << *b << ")";
  }
};

struct ExprLt : public ExprBinaryOp {
  inline ExprLt(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(L_EXPR_OP_LT, ty, a, b) {
    liong::assert(ty->cls == L_TYPE_CLASS_BOOL);
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "(" << *a << " < " << *b << ")";
  }
};
struct ExprEq : public ExprBinaryOp {
  inline ExprEq(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(L_EXPR_OP_EQ, ty, a, b) {
    liong::assert(ty->cls == L_TYPE_CLASS_BOOL);
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "(" << *a << " < " << *b << ")";
  }
};
struct ExprTypeCast : public Expr {
  std::shared_ptr<Expr> src;
  inline ExprTypeCast(
    const std::shared_ptr<Type>& dst_ty,
    const std::shared_ptr<Expr>& src
  ) : Expr(L_EXPR_OP_TYPE_CAST, dst_ty), src(src) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "(" << *src << ":" << *ty << ")";
  }
};
