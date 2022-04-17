// Universal visitor base type.
// @PENGUINLIONG
#pragma once
#include "spv/node.hpp"
#include "visitor/gen/mem-visitor.hpp"
#include "visitor/gen/type-visitor.hpp"
#include "visitor/gen/expr-visitor.hpp"
#include "visitor/gen/stmt-visitor.hpp"

struct Visitor :
  public MemoryVisitor,
  public TypeVisitor,
  public ExprVisitor,
  public StmtVisitor
{
  void visit(const Node& node) {
    switch (node.nova) {
    case L_NODE_VARIANT_MEMORY: return visit_mem(*(const Memory*)&node);
    case L_NODE_VARIANT_TYPE: return visit_ty(*(const Type*)&node);
    case L_NODE_VARIANT_EXPR: return visit_expr(*(const Expr*)&node);
    case L_NODE_VARIANT_STMT: return visit_stmt(*(const Stmt*)&node);
    default: liong::unimplemented();
    }
  }
  inline void visit(const Memory& mem) { visit_mem(mem); }
  inline void visit(const Type& ty) { visit_ty(ty); }
  inline void visit(const Expr& expr) { visit_expr(expr); }
  inline void visit(const Stmt& stmt) { visit_stmt(stmt); }
};
