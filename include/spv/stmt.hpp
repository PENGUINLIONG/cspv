// Statements op implementations.
// @PENGUINLIONG
#pragma once
#include "spv/stmt-reg.hpp"

struct StmtBlock : public Stmt {
  const std::vector<std::shared_ptr<Stmt>> stmts;

  inline StmtBlock(
    const std::vector<std::shared_ptr<Stmt>>& stmts
  ) : Stmt(L_STMT_OP_BLOCK), stmts(stmts) {}
  inline StmtBlock(
    std::vector<std::shared_ptr<Stmt>>&& stmts
  ) : Stmt(L_STMT_OP_BLOCK),
    stmts(std::forward<std::vector<std::shared_ptr<Stmt>>>(stmts)) {}
};

struct StmtConditionalBranch : public Stmt {
  std::shared_ptr<Expr> cond;
  std::shared_ptr<Stmt> then_block;
  std::shared_ptr<Stmt> else_block;

  inline StmtConditionalBranch(
    const std::shared_ptr<Expr>& cond,
    const std::shared_ptr<Stmt>& then_block,
    const std::shared_ptr<Stmt>& else_block
  ) : Stmt(L_STMT_OP_CONDITIONAL_BRANCH), cond(cond), then_block(then_block),
    else_block(else_block) {}
};

struct StmtIfThenElse : public Stmt {
  std::shared_ptr<Stmt> body_block;

  inline StmtIfThenElse(
    const std::shared_ptr<Stmt>& body_block
  ) : Stmt(L_STMT_OP_IF_THEN_ELSE), body_block(body_block) {}
};
struct StmtLoop : public Stmt {
  std::shared_ptr<Stmt> body_block;

  inline StmtLoop(
    const std::shared_ptr<Stmt>& body_block
  ) : Stmt(L_STMT_OP_LOOP), body_block(body_block) {}
};
struct StmtReturn : public Stmt {
  const std::shared_ptr<Expr> rv;

  inline StmtReturn(const std::shared_ptr<Expr>& rv) :
    Stmt(L_STMT_OP_RETURN), rv(rv) {}
};
struct StmtIfThenElseMerge : public Stmt {
  inline StmtIfThenElseMerge() : Stmt(L_STMT_OP_IF_THEN_ELSE_MERGE) {}
};
struct StmtLoopMerge : public Stmt {
  inline StmtLoopMerge() : Stmt(L_STMT_OP_LOOP_MERGE) {}
};
struct StmtLoopBackEdge : public Stmt {
    inline StmtLoopBackEdge() : Stmt(L_STMT_OP_LOOP_BACK_EDGE) {}
};



struct StmtStore : public Stmt {
  const std::shared_ptr<Memory> dst_ptr;
  const std::shared_ptr<Expr> value;

  inline StmtStore(
    const std::shared_ptr<Memory>& dst_ptr,
    const std::shared_ptr<Expr>& value
  ) : Stmt(L_STMT_OP_STORE), dst_ptr(dst_ptr), value(value) {}
};
