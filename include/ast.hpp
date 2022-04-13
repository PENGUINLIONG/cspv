// AST Infrastructure.
// @PENGUINLIONG
#pragma once
#include <memory>
#include <utility>
#include "gft/assert.hpp"
#include "spv/mod.hpp"

enum TypeClass {
  L_TYPE_CLASS_VOID,
  L_TYPE_CLASS_BOOL,
  L_TYPE_CLASS_INT,
  L_TYPE_CLASS_FLOAT,
  L_TYPE_CLASS_POINTER,
};
struct Type {
  const TypeClass cls;

  virtual bool is_same_as(const Type& other) const { return false; }

protected:
  inline Type(TypeClass cls) : cls(cls) {}
};
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



enum ExprOp {
  L_EXPR_OP_CONSTANT,
  L_EXPR_OP_VARIABLE,

  L_EXPR_OP_LOAD,

  L_EXPR_OP_NEG,
  L_EXPR_OP_ADD,
  L_EXPR_OP_SUB,
  L_EXPR_OP_MUL,
  L_EXPR_OP_DIV,
  L_EXPR_OP_MOD,

  L_EXPR_OP_LT,

  L_EXPR_OP_AND,
  L_EXPR_OP_OR,
  L_EXPR_OP_NOT,
  L_EXPR_OP_XOR,
};
struct Expr {
  const ExprOp op;
  const std::shared_ptr<Type> ty;

protected:
  inline Expr(ExprOp op, const std::shared_ptr<Type>& ty) : op(op), ty(ty) {}
};
struct ExprConstant : public Expr {
  const std::vector<uint32_t> lits;

  inline ExprConstant(
    const std::shared_ptr<Type>& ty,
    std::vector<uint32_t>&& lits
  ) : Expr(L_EXPR_OP_CONSTANT, ty), lits(lits) {}
};
struct ExprVariable : public Expr {
  spv::StorageClass store_cls;

  inline ExprVariable(
    const std::shared_ptr<Type>& ty,
    spv::StorageClass store_cls
  ) : Expr(L_EXPR_OP_VARIABLE, ty), store_cls(store_cls) {}
};

struct ExprLoad : public Expr {
  const std::shared_ptr<Expr> src_ptr;

  inline ExprLoad(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& src_ptr
  ) : Expr(L_EXPR_OP_LOAD, ty), src_ptr(src_ptr) {}
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
};
struct ExprSub : public ExprBinaryOp {
  inline ExprSub(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(L_EXPR_OP_SUB, ty, a, b) {
    liong::assert(ty->is_same_as(*a->ty));
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
};


enum StmtOp {
  L_STMT_OP_STORE,
};
struct Stmt {
  const StmtOp op;

protected:
  inline Stmt(StmtOp op) : op(op) {}
};
struct StmtStore : public Stmt {
  const std::shared_ptr<Expr> dst_ptr;
  const std::shared_ptr<Expr> value;

  inline StmtStore(
    const std::shared_ptr<Expr>& dst_ptr,
    const std::shared_ptr<Expr>& value
  ) : Stmt(L_STMT_OP_STORE), dst_ptr(dst_ptr), value(value) {}
};


struct ControlFlow;

enum BranchType {
  L_BRANCH_TYPE_NEVER,
  L_BRANCH_TYPE_CONDITION_THEN,
  L_BRANCH_TYPE_CONDITION_ELSE,
  L_BRANCH_TYPE_ALWAYS,
};
struct Branch {
  BranchType branch_ty;
  std::shared_ptr<Expr> cond;
  std::unique_ptr<ControlFlow> ctrl_flow;
};

struct ControlFlowSelection {
  std::vector<Branch> branches;
};
struct ControlFlowInitLoop {
  std::unique_ptr<ControlFlow> body;
};
struct ControlFlowLoop {
  std::shared_ptr<Expr> cond;
  std::unique_ptr<ControlFlow> body;
};
struct ControlFlowReturn {
  InstructionRef rv;
};
struct ControlFlow {
  InstructionRef label;
  std::unique_ptr<ControlFlow> next;
  std::vector<std::shared_ptr<Stmt>> stmts;
  // Extra parameters.
  std::unique_ptr<ControlFlowSelection> sel;
  std::unique_ptr<ControlFlowReturn> ret;
  std::unique_ptr<ControlFlowLoop> loop;
};

extern std::map<std::string, std::unique_ptr<ControlFlow>> extract_entry_points(const SpirvModule& mod);
