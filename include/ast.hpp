// AST Infrastructure.
// @PENGUINLIONG
#pragma once
#include <memory>
#include <utility>
#include "spv/instr.hpp"
#include "spv/abstr.hpp"

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
std::shared_ptr<Type> parse_ty(
  const SpirvAbstract& abstr,
  const InstructionRef& instr
);

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
std::shared_ptr<Expr> parse_expr(
  const SpirvAbstract& abstr,
  const InstructionRef& instr
);


enum StmtOp {
  L_STMT_OP_STORE,
};
struct Stmt {
  const StmtOp op;

protected:
  inline Stmt(StmtOp op) : op(op) {}
};
std::shared_ptr<Stmt> parse_stmt(
  const SpirvAbstract& abstr,
  const InstructionRef& instr
);
