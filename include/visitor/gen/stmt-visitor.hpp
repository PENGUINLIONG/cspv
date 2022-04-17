// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.
// Statement tree visitor.
// @PENGUINLIONG
#pragma once
#include "spv/stmt.hpp"

struct StmtVisitor {
  void visit_stmt(const Stmt& stmt) {
    switch (stmt.op) {
    case L_STMT_OP_BLOCK: visit_stmt(*(const StmtBlock*)&stmt); break;
    case L_STMT_OP_CONDITIONAL_BRANCH: visit_stmt(*(const StmtConditionalBranch*)&stmt); break;
    case L_STMT_OP_IF_THEN_ELSE: visit_stmt(*(const StmtIfThenElse*)&stmt); break;
    case L_STMT_OP_LOOP: visit_stmt(*(const StmtLoop*)&stmt); break;
    case L_STMT_OP_RETURN: visit_stmt(*(const StmtReturn*)&stmt); break;
    case L_STMT_OP_IF_THEN_ELSE_MERGE: visit_stmt(*(const StmtIfThenElseMerge*)&stmt); break;
    case L_STMT_OP_LOOP_MERGE: visit_stmt(*(const StmtLoopMerge*)&stmt); break;
    case L_STMT_OP_LOOP_CONTINUE: visit_stmt(*(const StmtLoopContinue*)&stmt); break;
    case L_STMT_OP_LOOP_BACK_EDGE: visit_stmt(*(const StmtLoopBackEdge*)&stmt); break;
    case L_STMT_OP_STORE: visit_stmt(*(const StmtStore*)&stmt); break;
    default: liong::unreachable();
    }
  }
  virtual void visit_stmt(const StmtBlock&) {}
  virtual void visit_stmt(const StmtConditionalBranch&) {}
  virtual void visit_stmt(const StmtIfThenElse&) {}
  virtual void visit_stmt(const StmtLoop&) {}
  virtual void visit_stmt(const StmtReturn&) {}
  virtual void visit_stmt(const StmtIfThenElseMerge&) {}
  virtual void visit_stmt(const StmtLoopMerge&) {}
  virtual void visit_stmt(const StmtLoopContinue&) {}
  virtual void visit_stmt(const StmtLoopBackEdge&) {}
  virtual void visit_stmt(const StmtStore&) {}
};
