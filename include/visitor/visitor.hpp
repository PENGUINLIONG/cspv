// Universal visitor base type.
// @PENGUINLIONG
#pragma once
#include "node/node.hpp"
#include "visitor/gen/mem-visitor.hpp"
#include "visitor/gen/ty-visitor.hpp"
#include "visitor/gen/expr-visitor.hpp"
#include "visitor/gen/stmt-visitor.hpp"

struct Visitor :
  public MemoryVisitor,
  public TypeVisitor,
  public ExprVisitor,
  public StmtVisitor
{
  void visit(const NodeRef& node) {
    switch (node->nova) {
    case L_NODE_VARIANT_MEMORY: return visit_mem(std::static_pointer_cast<Memory>(node));
    case L_NODE_VARIANT_TYPE: return visit_ty(std::static_pointer_cast<Type>(node));
    case L_NODE_VARIANT_EXPR: return visit_expr(std::static_pointer_cast<Expr>(node));
    case L_NODE_VARIANT_STMT: return visit_stmt(std::static_pointer_cast<Stmt>(node));
    default: liong::unimplemented();
    }
  }
  inline void visit(const MemoryRef& mem) { visit_mem(mem); }
  inline void visit(const TypeRef& ty) { visit_ty(ty); }
  inline void visit(const ExprRef& expr) { visit_expr(expr); }
  inline void visit(const StmtRef& stmt) { visit_stmt(stmt); }
};
