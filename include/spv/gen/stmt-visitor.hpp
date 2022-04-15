// GENERATED SOURCE; DO NOT MODIFY.
// Statement tree visitor.
// @PENGUINLIONG
#pragma once
#include "spv/stmt.hpp"

struct StmtVisitor {
  void visit_stmt(const Stmt& stmt) {
    switch (stmt.op) {
    case L_STMT_OP_STORE: stmt.visit(this); visit_stmt(*(const StmtStore*)&stmt); break;
    default: liong::unreachable();
    }
  }
  virtual void visit_stmt(const StmtStore&) {}
};
