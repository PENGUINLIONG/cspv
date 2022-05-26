// Low-level stateful instructions.
// @PENGUINLIONG
#pragma once
#include "mem.hpp"

namespace lo {

typedef std::shared_ptr<struct Instr> InstrRef;
typedef uint32_t InstrClass;

struct Block {
  struct Inner {
    std::vector<InstrRef> instrs;
  };
  std::unique_ptr<Inner> inner;
};
typedef std::shared_ptr<Instr> BlockRef;

struct Instr {
  const InstrClass cls;

  inline Instr(InstrClass cls) : cls(cls) {}

  template<typename T>
  inline as() { return static_cast<T&>(this); }
  template<typename T>
  inline as() const { return static_cast<const T&>(this); }
};
template<typename T>
struct Instr_ : public Instr {
  static const InstrClass CLS = Counter<Instr>::get_id();
  Instr_() : Instr(CLS) {}
};

struct InstrNop : public Instr_<InstrNop> {
};
struct InstrIntImm : public Instr_<InstrIntImm> {
  int64_t lit;
  MemoryRef dst;
};
struct InstrFloatImm : public Instr_<InstrFloatImm> {
  double lit;
  MemoryRef dst;
};
struct InstrCopy : public Instr_<InstrCopy> {
  MemoryRef src;
  MemoryRef dst;
};
// Arithmetic, logic and elementary function op.
struct InstrAle : public Instr_<InstrAle> {
  enum Op {
    L_ALE_OP_ADD,
    L_ALE_OP_SUB,
    L_ALE_OP_MUL,
    L_ALE_OP_DIV,
    L_ALE_OP_MOD,
    L_ALE_OP_SELECT,
  };
  Op ale_op;
  MemoryRef a;
  MemoryRef b;
  MemoryRef c;
  MemoryRef dst;
};
struct InstrAccessChain : public Instr_<InstrAccessChain> {
  MemoryRef ptr;
  uint32_t i;
  MemoryRef dst;
};
struct InstrJump : public Instr_<InstrJump> {
  BlockRef target;
};
struct InstrBranch : public Instr_<InstrBranch> {
  MemoryRef cond;
  BlockRef then_branch;
  BlockRef else_branch;
};
struct InstrReturn : public Instr_<InstrReturn> {
};
struct InstrReturnValue : public Instr_<InstrReturnValue> {
  MemoryRef value;
};
struct InstrPanic : public Instr_<InstrPanic> {
};

} // namespace lo
