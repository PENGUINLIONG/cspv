// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.
// Statement node visitor.
// @PENGUINLIONG
#pragma once
#include "node/gen/stmt.hpp"

typedef std::shared_ptr<Stmt> StmtRef;
typedef std::shared_ptr<StmtBlock> StmtBlockRef;
typedef std::shared_ptr<StmtConditionalBranch> StmtConditionalBranchRef;
typedef std::shared_ptr<StmtIfThenElse> StmtIfThenElseRef;
typedef std::shared_ptr<StmtLoop> StmtLoopRef;
typedef std::shared_ptr<StmtReturn> StmtReturnRef;
typedef std::shared_ptr<StmtIfThenElseMerge> StmtIfThenElseMergeRef;
typedef std::shared_ptr<StmtLoopMerge> StmtLoopMergeRef;
typedef std::shared_ptr<StmtLoopContinue> StmtLoopContinueRef;
typedef std::shared_ptr<StmtLoopBackEdge> StmtLoopBackEdgeRef;
typedef std::shared_ptr<StmtRangedLoop> StmtRangedLoopRef;
typedef std::shared_ptr<StmtStore> StmtStoreRef;

struct StmtVisitor {
  virtual void visit_stmt(const Stmt& stmt) {
    switch (stmt.op) {
    case L_STMT_OP_BLOCK: visit_stmt_(*(const StmtBlock*)&stmt); break;
    case L_STMT_OP_CONDITIONAL_BRANCH: visit_stmt_(*(const StmtConditionalBranch*)&stmt); break;
    case L_STMT_OP_IF_THEN_ELSE: visit_stmt_(*(const StmtIfThenElse*)&stmt); break;
    case L_STMT_OP_LOOP: visit_stmt_(*(const StmtLoop*)&stmt); break;
    case L_STMT_OP_RETURN: visit_stmt_(*(const StmtReturn*)&stmt); break;
    case L_STMT_OP_IF_THEN_ELSE_MERGE: visit_stmt_(*(const StmtIfThenElseMerge*)&stmt); break;
    case L_STMT_OP_LOOP_MERGE: visit_stmt_(*(const StmtLoopMerge*)&stmt); break;
    case L_STMT_OP_LOOP_CONTINUE: visit_stmt_(*(const StmtLoopContinue*)&stmt); break;
    case L_STMT_OP_LOOP_BACK_EDGE: visit_stmt_(*(const StmtLoopBackEdge*)&stmt); break;
    case L_STMT_OP_RANGED_LOOP: visit_stmt_(*(const StmtRangedLoop*)&stmt); break;
    case L_STMT_OP_STORE: visit_stmt_(*(const StmtStore*)&stmt); break;
    default: liong::unreachable();
    }
  }
  virtual void visit_stmt_(const StmtBlock&);
  virtual void visit_stmt_(const StmtConditionalBranch&);
  virtual void visit_stmt_(const StmtIfThenElse&);
  virtual void visit_stmt_(const StmtLoop&);
  virtual void visit_stmt_(const StmtReturn&);
  virtual void visit_stmt_(const StmtIfThenElseMerge&);
  virtual void visit_stmt_(const StmtLoopMerge&);
  virtual void visit_stmt_(const StmtLoopContinue&);
  virtual void visit_stmt_(const StmtLoopBackEdge&);
  virtual void visit_stmt_(const StmtRangedLoop&);
  virtual void visit_stmt_(const StmtStore&);
};

template<typename TStmt>
struct StmtFunctorVisitor : public StmtVisitor {
  std::function<void(const TStmt&)> f;
  StmtFunctorVisitor(std::function<void(const TStmt&)>&& f) :
    f(std::forward<std::function<void(const TStmt&)>>(f)) {}

  virtual void visit_stmt_(const TStmt& stmt) override final {
    f(stmt);
  }
};
template<typename TStmt>
void visit_stmt_functor(
  std::function<void(const TStmt&)>&& f,
  const Stmt& x
) {
  StmtFunctorVisitor<TStmt> visitor(
    std::forward<std::function<void(const TStmt&)>>(f));
  visitor.visit_stmt(x);
}

struct StmtMutator {
  virtual StmtRef mutate_stmt(StmtRef& stmt) {
    switch (stmt->op) {
    case L_STMT_OP_BLOCK: return mutate_stmt_(std::static_pointer_cast<StmtBlock>(stmt));
    case L_STMT_OP_CONDITIONAL_BRANCH: return mutate_stmt_(std::static_pointer_cast<StmtConditionalBranch>(stmt));
    case L_STMT_OP_IF_THEN_ELSE: return mutate_stmt_(std::static_pointer_cast<StmtIfThenElse>(stmt));
    case L_STMT_OP_LOOP: return mutate_stmt_(std::static_pointer_cast<StmtLoop>(stmt));
    case L_STMT_OP_RETURN: return mutate_stmt_(std::static_pointer_cast<StmtReturn>(stmt));
    case L_STMT_OP_IF_THEN_ELSE_MERGE: return mutate_stmt_(std::static_pointer_cast<StmtIfThenElseMerge>(stmt));
    case L_STMT_OP_LOOP_MERGE: return mutate_stmt_(std::static_pointer_cast<StmtLoopMerge>(stmt));
    case L_STMT_OP_LOOP_CONTINUE: return mutate_stmt_(std::static_pointer_cast<StmtLoopContinue>(stmt));
    case L_STMT_OP_LOOP_BACK_EDGE: return mutate_stmt_(std::static_pointer_cast<StmtLoopBackEdge>(stmt));
    case L_STMT_OP_RANGED_LOOP: return mutate_stmt_(std::static_pointer_cast<StmtRangedLoop>(stmt));
    case L_STMT_OP_STORE: return mutate_stmt_(std::static_pointer_cast<StmtStore>(stmt));
    default: liong::unreachable();
    }
  }
  virtual StmtRef mutate_stmt_(std::shared_ptr<StmtBlock>&);
  virtual StmtRef mutate_stmt_(std::shared_ptr<StmtConditionalBranch>&);
  virtual StmtRef mutate_stmt_(std::shared_ptr<StmtIfThenElse>&);
  virtual StmtRef mutate_stmt_(std::shared_ptr<StmtLoop>&);
  virtual StmtRef mutate_stmt_(std::shared_ptr<StmtReturn>&);
  virtual StmtRef mutate_stmt_(std::shared_ptr<StmtIfThenElseMerge>&);
  virtual StmtRef mutate_stmt_(std::shared_ptr<StmtLoopMerge>&);
  virtual StmtRef mutate_stmt_(std::shared_ptr<StmtLoopContinue>&);
  virtual StmtRef mutate_stmt_(std::shared_ptr<StmtLoopBackEdge>&);
  virtual StmtRef mutate_stmt_(std::shared_ptr<StmtRangedLoop>&);
  virtual StmtRef mutate_stmt_(std::shared_ptr<StmtStore>&);
};

template<typename TStmt>
struct StmtFunctorMutator : public StmtMutator {
  typedef std::shared_ptr<TStmt> TStmtRef;
  std::function<StmtRef(TStmtRef&)> f;
  StmtFunctorMutator(std::function<StmtRef(TStmtRef&)>&& f) :
    f(std::forward<std::function<StmtRef(TStmtRef&)>>(f)) {}

  virtual StmtRef mutate_stmt_(TStmtRef& stmt) override final {
    return f(stmt);
  }
};
template<typename TStmt>
void mutate_stmt_functor(
  std::function<StmtRef(std::shared_ptr<TStmt>&)>&& f,
  const Stmt& x
) {
  StmtFunctorMutator<TStmt> mutator(
    std::forward<std::function<StmtRef(std::shared_ptr<TStmt>&)>>(f));
  return mutator.mutate_stmt(x);
}
