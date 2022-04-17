// Statements op implementations.
// @PENGUINLIONG
#pragma once
#include <memory>
#include <vector>
#include "spv/stmt-reg.hpp"
#include "spv/expr-reg.hpp"
#include "spv/mem-reg.hpp"

struct StmtBlock : public Stmt {
  static const StmtOp OP = L_STMT_OP_BLOCK;
  std::vector<std::shared_ptr<Stmt>> stmts;

  inline StmtBlock(
    const std::vector<std::shared_ptr<Stmt>>& stmts
  ) : Stmt(OP), stmts(stmts) {}
  inline StmtBlock(
    std::vector<std::shared_ptr<Stmt>>&& stmts
  ) : Stmt(OP),
    stmts(std::forward<std::vector<std::shared_ptr<Stmt>>>(stmts)) {}
};

struct StmtConditionalBranch : public Stmt {
  static const StmtOp OP = L_STMT_OP_CONDITIONAL_BRANCH;
  std::shared_ptr<Expr> cond;
  std::shared_ptr<Stmt> then_block;
  std::shared_ptr<Stmt> else_block;

  inline StmtConditionalBranch(
    const std::shared_ptr<Expr>& cond,
    const std::shared_ptr<Stmt>& then_block,
    const std::shared_ptr<Stmt>& else_block
  ) : Stmt(OP), cond(cond), then_block(then_block), else_block(else_block) {}
};

struct StmtIfThenElse : public Stmt {
  static const StmtOp OP = L_STMT_OP_IF_THEN_ELSE;
  std::shared_ptr<Stmt> body_block;

  inline StmtIfThenElse(
    const std::shared_ptr<Stmt>& body_block
  ) : Stmt(OP), body_block(body_block) {}
};
struct StmtLoop : public Stmt {
  static const StmtOp OP = L_STMT_OP_LOOP;
  std::shared_ptr<Stmt> body_block;
  std::shared_ptr<Stmt> continue_block;

  inline StmtLoop(
    const std::shared_ptr<Stmt>& body_block,
    const std::shared_ptr<Stmt>& continue_block
  ) : Stmt(OP), body_block(body_block), continue_block(continue_block) {}
};
struct StmtReturn : public Stmt {
  static const StmtOp OP = L_STMT_OP_RETURN;
  std::shared_ptr<Expr> rv;

  inline StmtReturn(const std::shared_ptr<Expr>& rv) : Stmt(OP), rv(rv) {}
};
struct StmtIfThenElseMerge : public Stmt {
  static const StmtOp OP = L_STMT_OP_IF_THEN_ELSE_MERGE;
  inline StmtIfThenElseMerge() : Stmt(OP) {}
};
struct StmtLoopMerge : public Stmt {
  static const StmtOp OP = L_STMT_OP_LOOP_MERGE;
  inline StmtLoopMerge() : Stmt(OP) {}
};
struct StmtLoopContinue : public Stmt {
  static const StmtOp OP = L_STMT_OP_LOOP_CONTINUE;
  inline StmtLoopContinue() : Stmt(OP) {}
};
struct StmtLoopBackEdge : public Stmt {
  static const StmtOp OP = L_STMT_OP_LOOP_BACK_EDGE;
  inline StmtLoopBackEdge() : Stmt(OP) {}
};


struct StmtRangedLoop : public Stmt {
  static const StmtOp OP = L_STMT_OP_RANGED_LOOP;
  std::shared_ptr<Stmt> body_block;
  std::shared_ptr<Memory> itervar;
  std::shared_ptr<Expr> begin;
  std::shared_ptr<Expr> end;
  std::shared_ptr<Expr> stride;

  inline StmtRangedLoop(
    const std::shared_ptr<Stmt>& body_block,
    const std::shared_ptr<Memory>& itervar,
    const std::shared_ptr<Expr>& begin,
    const std::shared_ptr<Expr>& end,
    const std::shared_ptr<Expr>& stride
  ) : Stmt(OP), body_block(body_block), itervar(itervar), begin(begin),
    end(end), stride(stride) {}
};



struct StmtStore : public Stmt {
  static const StmtOp OP = L_STMT_OP_STORE;
  const std::shared_ptr<Memory> dst_ptr;
  const std::shared_ptr<Expr> value;

  inline StmtStore(
    const std::shared_ptr<Memory>& dst_ptr,
    const std::shared_ptr<Expr>& value
  ) : Stmt(OP), dst_ptr(dst_ptr), value(value) {}
};
