// Control-flow class registry.
// @PENGUINLIONG
#pragma once
#include "spv/stmt-reg.hpp"

enum ControlFlowClass {
  L_CONTROL_FLOW_CLASS_EXECUTABLE,
  L_CONTROL_FLOW_CLASS_JUMP,
  L_CONTROL_FLOW_CLASS_SELECT,
  L_CONTROL_FLOW_CLASS_LOOP,
  L_CONTROL_FLOW_CLASS_RETURN,
  L_CONTROL_FLOW_CLASS_BACK_EDGE,
  L_CONTROL_FLOW_CLASS_MERGE_SELECT,
  L_CONTROL_FLOW_CLASS_MERGE_LOOP,
};
struct ControlFlow {
  ControlFlowClass cls;
  const InstructionRef label;
  const std::unique_ptr<ControlFlow> next;

  virtual void dbg_print(Debug& s) const { s << "ctrlflow?"; }

protected:
  inline ControlFlow(
    ControlFlowClass cls,
    const InstructionRef& label,
    std::unique_ptr<ControlFlow>&& next
  ) : label(label), next(std::forward<std::unique_ptr<ControlFlow>>(next)) {}
};

inline Debug& operator<<(Debug& s, const ControlFlow& x) {
  x.dbg_print(s);
  return s;
}
