// Statements op registry.
// @PENGUINLIONG
#pragma once
#include "spv/node.hpp"

enum StmtOp {
  L_STMT_OP_BLOCK,
  L_STMT_OP_CONDITIONAL_BRANCH,
  L_STMT_OP_IF_THEN_ELSE,
  L_STMT_OP_LOOP,
  L_STMT_OP_RETURN,
  L_STMT_OP_IF_THEN_ELSE_MERGE,
  L_STMT_OP_LOOP_MERGE,
  L_STMT_OP_LOOP_BACK_EDGE,

  L_STMT_OP_STORE,
};
struct Stmt : public Node {
  const StmtOp op;

protected:
  inline Stmt(StmtOp op) : Node(L_NODE_VARIANT_STMT), op(op) {}
};
