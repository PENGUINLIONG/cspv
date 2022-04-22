// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.
// Stmt node implementation.
// @PENGUINLIONG
#pragma once
#include "node/reg.hpp"

struct StmtNop : public Stmt {
  static const StmtOp OP = L_STMT_OP_NOP;

  inline StmtNop(
  ) : Stmt(L_STMT_OP_NOP) {
  }

  virtual void collect_children(NodeDrain* drain) override final {
  }
};

struct StmtBlock : public Stmt {
  static const StmtOp OP = L_STMT_OP_BLOCK;
  std::vector<NodeRef<Stmt>> stmts;

  inline StmtBlock(
    const std::vector<NodeRef<Stmt>>& stmts
  ) : Stmt(L_STMT_OP_BLOCK), stmts(stmts) {
    for (const auto& x : stmts) { liong::assert(x != nullptr); }
  }

  virtual void collect_children(NodeDrain* drain) override final {
    for (const auto& x : stmts) { drain->push(x); }
  }
};

struct StmtConditional : public Stmt {
  static const StmtOp OP = L_STMT_OP_CONDITIONAL;
  NodeRef<Expr> cond;
  NodeRef<Stmt> then_block;

  inline StmtConditional(
    const NodeRef<Expr>& cond,
    const NodeRef<Stmt>& then_block
  ) : Stmt(L_STMT_OP_CONDITIONAL), cond(cond), then_block(then_block) {
    liong::assert(cond != nullptr);
    liong::assert(then_block != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) override final {
    drain->push(cond);
    drain->push(then_block);
  }
};

struct StmtConditionalBranch : public Stmt {
  static const StmtOp OP = L_STMT_OP_CONDITIONAL_BRANCH;
  NodeRef<Expr> cond;
  NodeRef<Stmt> then_block;
  NodeRef<Stmt> else_block;

  inline StmtConditionalBranch(
    const NodeRef<Expr>& cond,
    const NodeRef<Stmt>& then_block,
    const NodeRef<Stmt>& else_block
  ) : Stmt(L_STMT_OP_CONDITIONAL_BRANCH), cond(cond), then_block(then_block), else_block(else_block) {
    liong::assert(cond != nullptr);
    liong::assert(then_block != nullptr);
    liong::assert(else_block != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) override final {
    drain->push(cond);
    drain->push(then_block);
    drain->push(else_block);
  }
};

struct StmtIfThenElse : public Stmt {
  static const StmtOp OP = L_STMT_OP_IF_THEN_ELSE;
  NodeRef<Stmt> body_block;
  void* handle;

  inline StmtIfThenElse(
    const NodeRef<Stmt>& body_block,
    void* handle
  ) : Stmt(L_STMT_OP_IF_THEN_ELSE), body_block(body_block), handle(handle) {
    liong::assert(body_block != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) override final {
    drain->push(body_block);
  }
};

struct StmtLoop : public Stmt {
  static const StmtOp OP = L_STMT_OP_LOOP;
  NodeRef<Stmt> body_block;
  NodeRef<Stmt> continue_block;
  void* handle;

  inline StmtLoop(
    const NodeRef<Stmt>& body_block,
    const NodeRef<Stmt>& continue_block,
    void* handle
  ) : Stmt(L_STMT_OP_LOOP), body_block(body_block), continue_block(continue_block), handle(handle) {
    liong::assert(body_block != nullptr);
    liong::assert(continue_block != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) override final {
    drain->push(body_block);
    drain->push(continue_block);
  }
};

struct StmtReturn : public Stmt {
  static const StmtOp OP = L_STMT_OP_RETURN;

  inline StmtReturn(
  ) : Stmt(L_STMT_OP_RETURN) {
  }

  virtual void collect_children(NodeDrain* drain) override final {
  }
};

struct StmtIfThenElseMerge : public Stmt {
  static const StmtOp OP = L_STMT_OP_IF_THEN_ELSE_MERGE;
  void* handle;

  inline StmtIfThenElseMerge(
    void* handle
  ) : Stmt(L_STMT_OP_IF_THEN_ELSE_MERGE), handle(handle) {
  }

  virtual void collect_children(NodeDrain* drain) override final {
  }
};

struct StmtLoopMerge : public Stmt {
  static const StmtOp OP = L_STMT_OP_LOOP_MERGE;
  void* handle;

  inline StmtLoopMerge(
    void* handle
  ) : Stmt(L_STMT_OP_LOOP_MERGE), handle(handle) {
  }

  virtual void collect_children(NodeDrain* drain) override final {
  }
};

struct StmtLoopContinue : public Stmt {
  static const StmtOp OP = L_STMT_OP_LOOP_CONTINUE;
  void* handle;

  inline StmtLoopContinue(
    void* handle
  ) : Stmt(L_STMT_OP_LOOP_CONTINUE), handle(handle) {
  }

  virtual void collect_children(NodeDrain* drain) override final {
  }
};

struct StmtLoopBackEdge : public Stmt {
  static const StmtOp OP = L_STMT_OP_LOOP_BACK_EDGE;
  void* handle;

  inline StmtLoopBackEdge(
    void* handle
  ) : Stmt(L_STMT_OP_LOOP_BACK_EDGE), handle(handle) {
  }

  virtual void collect_children(NodeDrain* drain) override final {
  }
};

struct StmtRangedLoop : public Stmt {
  static const StmtOp OP = L_STMT_OP_RANGED_LOOP;
  NodeRef<Stmt> body_block;
  NodeRef<Memory> itervar;

  inline StmtRangedLoop(
    const NodeRef<Stmt>& body_block,
    const NodeRef<Memory>& itervar
  ) : Stmt(L_STMT_OP_RANGED_LOOP), body_block(body_block), itervar(itervar) {
    liong::assert(body_block != nullptr);
    liong::assert(itervar != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) override final {
    drain->push(body_block);
    drain->push(itervar);
  }
};

struct StmtStore : public Stmt {
  static const StmtOp OP = L_STMT_OP_STORE;
  NodeRef<Memory> dst_ptr;
  NodeRef<Expr> value;

  inline StmtStore(
    const NodeRef<Memory>& dst_ptr,
    const NodeRef<Expr>& value
  ) : Stmt(L_STMT_OP_STORE), dst_ptr(dst_ptr), value(value) {
    liong::assert(dst_ptr != nullptr);
    liong::assert(value != nullptr);
  }

  virtual void collect_children(NodeDrain* drain) override final {
    drain->push(dst_ptr);
    drain->push(value);
  }
};

typedef NodeRef<Stmt> StmtRef;
typedef NodeRef<StmtNop> StmtNopRef;
typedef NodeRef<StmtBlock> StmtBlockRef;
typedef NodeRef<StmtConditional> StmtConditionalRef;
typedef NodeRef<StmtConditionalBranch> StmtConditionalBranchRef;
typedef NodeRef<StmtIfThenElse> StmtIfThenElseRef;
typedef NodeRef<StmtLoop> StmtLoopRef;
typedef NodeRef<StmtReturn> StmtReturnRef;
typedef NodeRef<StmtIfThenElseMerge> StmtIfThenElseMergeRef;
typedef NodeRef<StmtLoopMerge> StmtLoopMergeRef;
typedef NodeRef<StmtLoopContinue> StmtLoopContinueRef;
typedef NodeRef<StmtLoopBackEdge> StmtLoopBackEdgeRef;
typedef NodeRef<StmtRangedLoop> StmtRangedLoopRef;
typedef NodeRef<StmtStore> StmtStoreRef;
