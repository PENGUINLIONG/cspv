// Contorl-flow implementations.
// @PENGUINLIONG
#pragma once
#include "spv/ctrl-flow-reg.hpp"

enum BranchType {
  L_BRANCH_TYPE_NEVER,
  L_BRANCH_TYPE_CONDITION_THEN,
  L_BRANCH_TYPE_CONDITION_ELSE,
  L_BRANCH_TYPE_ALWAYS,
};
struct Branch {
  BranchType branch_ty;
  std::shared_ptr<Expr> cond;
  std::unique_ptr<ControlFlow> ctrl_flow;

  inline void dbg_print_cond(Debug& s) const {
    if (branch_ty == L_BRANCH_TYPE_ALWAYS) {
      s << "(true)";
    } else if (branch_ty == L_BRANCH_TYPE_NEVER) {
      s << "(false)";
    } else if (branch_ty == L_BRANCH_TYPE_CONDITION_THEN) {
      s << *cond;
    } else if (branch_ty == L_BRANCH_TYPE_CONDITION_ELSE) {
      s << "(!" << *cond << ")";
    } else {
      liong::unreachable();
    }
  }
};

struct ControlFlowExecutable : public ControlFlow {
  const std::vector<std::shared_ptr<Stmt>> stmts;

  inline ControlFlowExecutable(
    const InstructionRef& label,
    std::unique_ptr<ControlFlow>&& next,
    std::vector<std::shared_ptr<Stmt>>&& stmts
  ) : ControlFlow(L_CONTROL_FLOW_CLASS_EXECUTABLE, label,
    std::forward<std::unique_ptr<ControlFlow>>(next)),
    stmts(std::forward<std::vector<std::shared_ptr<Stmt>>>(stmts)) {}

  virtual void dbg_print(Debug& s) const override final {
    for (const auto& stmt : stmts) {
      s << *stmt << std::endl;
    }
    s << *next;
  }
};

struct ControlFlowJump : public ControlFlow {
  inline ControlFlowJump(
    const InstructionRef& label,
    std::unique_ptr<ControlFlow>&& next
  ) : ControlFlow(L_CONTROL_FLOW_CLASS_SELECT, label,
    std::forward<std::unique_ptr<ControlFlow>>(next)) {}

  virtual void dbg_print(Debug& s) const override final {
    s << *next;
  }
};
struct ControlFlowSelect : public ControlFlow {
  const std::vector<Branch> branches;

  inline ControlFlowSelect(
    const InstructionRef& label,
    std::unique_ptr<ControlFlow>&& next,
    std::vector<Branch>&& branches
  ) : ControlFlow(L_CONTROL_FLOW_CLASS_SELECT, label,
    std::forward<std::unique_ptr<ControlFlow>>(next)),
    branches(std::forward<std::vector<Branch>>(branches)) {}

  virtual void dbg_print(Debug& s) const override final {
    for (const auto& branch : branches) {
      s << "if ";
      branch.dbg_print_cond(s);
      s << " {" << std::endl;
      s.push_indent();
      s << *branch.ctrl_flow;
      s.pop_indent();
      s << "}" << std::endl;
    }
    s << *next;
  }
};
struct ControlFlowLoop : public ControlFlow {
  const Branch body;

  inline ControlFlowLoop(
    const InstructionRef& label,
    std::unique_ptr<ControlFlow>&& next,
    Branch&& body
  ) : ControlFlow(L_CONTROL_FLOW_CLASS_LOOP, label,
    std::forward<std::unique_ptr<ControlFlow>>(next)),
    body(std::forward<Branch>(body)) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "while ";
    body.dbg_print_cond(s);
    s << " {" << std::endl;
    s.push_indent();
    s << *body.ctrl_flow;
    s.pop_indent();
    s << "}" << std::endl;
    s << *next;
  }
};
struct ControlFlowReturn : public ControlFlow {
  const std::shared_ptr<Expr> rv;

  inline ControlFlowReturn(
    const InstructionRef& label,
    const std::shared_ptr<Expr>& rv
  ) : ControlFlow(L_CONTROL_FLOW_CLASS_RETURN, label, {}), rv(rv) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "return";
    if (rv) {
      s << " " << *rv;
    }
    s << std::endl;
  }
};
struct ControlFlowBackEdge : public ControlFlow {
    inline ControlFlowBackEdge(
        const InstructionRef& label
    ) : ControlFlow(L_CONTROL_FLOW_CLASS_BACK_EDGE, label, {}) {}

    virtual void dbg_print(Debug& s) const override final {}
};
struct ControlFlowMergeSelect : public ControlFlow {
  inline ControlFlowMergeSelect(
    const InstructionRef& label
  ) : ControlFlow(L_CONTROL_FLOW_CLASS_MERGE_SELECT, label, {}) {}

  virtual void dbg_print(Debug& s) const override final {}
};
struct ControlFlowMergeLoop : public ControlFlow {
  inline ControlFlowMergeLoop(
    const InstructionRef& label
  ) : ControlFlow(L_CONTROL_FLOW_CLASS_MERGE_LOOP, label, {}) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "break" << std::endl;
  }
};
