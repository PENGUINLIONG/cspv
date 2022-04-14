// AST Infrastructure.
// @PENGUINLIONG
#pragma once
#include <memory>
#include <ostream>
#include <utility>
#include "gft/assert.hpp"
#include "spv/mod.hpp"
#include "spv/dbg.hpp"

enum TypeClass {
  L_TYPE_CLASS_VOID,
  L_TYPE_CLASS_BOOL,
  L_TYPE_CLASS_INT,
  L_TYPE_CLASS_FLOAT,
  L_TYPE_CLASS_STRUCT,
  L_TYPE_CLASS_POINTER,
};
struct Type {
  const TypeClass cls;

  virtual bool is_same_as(const Type& other) const {
    liong::unimplemented();
  }

  virtual void dbg_print(Debug& s) const { s << "type?"; }

protected:
  inline Type(TypeClass cls) : cls(cls) {}
};



enum ExprOp {
  L_EXPR_OP_CONSTANT,

  L_EXPR_OP_LOAD,

  L_EXPR_OP_NEG,
  L_EXPR_OP_ADD,
  L_EXPR_OP_SUB,
  L_EXPR_OP_MUL,
  L_EXPR_OP_DIV,
  L_EXPR_OP_MOD,

  L_EXPR_OP_LT,
  L_EXPR_OP_EQ,

  L_EXPR_OP_AND,
  L_EXPR_OP_OR,
  L_EXPR_OP_NOT,
  L_EXPR_OP_XOR,

  L_EXPR_OP_TYPE_CAST,
};
struct Expr {
  const ExprOp op;
  const std::shared_ptr<Type> ty;

  virtual bool is_same_as(const Type& other) const {
    liong::unimplemented();
  }

  virtual void dbg_print(Debug& s) const { s << "expr?"; }

protected:
  inline Expr(ExprOp op, const std::shared_ptr<Type>& ty) : op(op), ty(ty) {}
};



struct AccessChain {
  std::vector<std::shared_ptr<Expr>> idxs;
};



enum MemoryClass {
  L_MEMORY_CLASS_FUNCTION_VARIABLE,
  L_MEMORY_CLASS_UNIFORM_BUFFER,
  L_MEMORY_CLASS_STORAGE_BUFFER,
  L_MEMORY_CLASS_SAMPLED_IMAGE,
  L_MEMORY_CLASS_STORAGE_IMAGE,
};
struct Memory {
  const MemoryClass cls;
  const std::shared_ptr<Type> ty;
  const AccessChain ac;

  virtual void dbg_print(Debug& s) const { s << "memory?"; }

protected:
  Memory(
    MemoryClass cls,
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac
  ) : cls(cls), ty(ty), ac(std::forward<AccessChain>(ac)) {}
};



enum StmtOp {
  L_STMT_OP_STORE,
};
struct Stmt {
  const StmtOp op;

  virtual bool is_same_as(const Type& other) const {
    liong::unimplemented();
  }

  virtual void dbg_print(Debug& s) const { s << "stmt?"; }

protected:
  inline Stmt(StmtOp op) : op(op) {}
};



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



inline Debug& operator<<(Debug& s, const Type& x) {
  x.dbg_print(s);
  return s;
}
inline Debug& operator<<(Debug& s, const Expr& x) {
  x.dbg_print(s);
  return s;
}
inline Debug& operator<<(Debug& s, const AccessChain& x) {
  bool first = true;
  for (const auto& idx : x.idxs) {
    if (first) {
      first = false;
    } else {
      s << ",";
    }
    s << *idx;
  }
  return s;
}
inline Debug& operator<<(Debug& s, const Memory& x) {
  x.dbg_print(s);
  return s;
}
inline Debug& operator<<(Debug& s, const Stmt& x) {
  x.dbg_print(s);
  return s;
}
inline Debug& operator<<(Debug& s, const ControlFlow& x) {
  x.dbg_print(s);
  return s;
}








struct TypeVoid : public Type {
  TypeVoid() : Type(L_TYPE_CLASS_VOID) {}
  virtual bool is_same_as(const Type& other) const override final {
    return other.cls == cls;
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "void";
  }
};
struct TypeBool : public Type {
  TypeBool() : Type(L_TYPE_CLASS_BOOL) {}
  virtual bool is_same_as(const Type& other) const override final {
    return other.cls == cls;
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "bool";
  }
};
struct TypeInt : public Type {
  uint32_t nbit;
  bool is_signed;
  TypeInt(uint32_t nbit, bool is_signed) : Type(L_TYPE_CLASS_INT),
    nbit(nbit), is_signed(is_signed) {}
  virtual bool is_same_as(const Type& other) const override final {
    if (other.cls != cls) { return false; }
    const auto& other2 = (const TypeInt&)other;
    return other2.nbit == nbit && other2.is_signed == is_signed;
  }

  virtual void dbg_print(Debug& s) const override final {
    s << (is_signed ? "i" : "u") << nbit;
  }
};
struct TypeFloat : public Type {
  uint32_t nbit;
  TypeFloat(uint32_t nbit) : Type(L_TYPE_CLASS_FLOAT), nbit(nbit) {}
  virtual bool is_same_as(const Type& other) const override final {
    if (other.cls != cls) { return false; }
    const auto& other2 = (const TypeFloat&)other;
    return other2.nbit == nbit;
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "f" << nbit;
  }
};
// For structs specifically, we assume that the memory layout for uniform
// buffers are in `std140` and storage buffers in `std430`. The layout depends
// on where the struct type is used.
struct TypeStruct : public Type {
  std::vector<std::shared_ptr<Type>> members;
  TypeStruct(
    std::vector<std::shared_ptr<Type>>&& members
  ) : Type(L_TYPE_CLASS_STRUCT),
    members(std::forward<std::vector<std::shared_ptr<Type>>>(members)) {}
  virtual bool is_same_as(const Type& other) const override final {
    if (other.cls != cls) { return false; }
    const auto& other2 = (const TypeStruct&)other;
    for (size_t i = 0; i < members.size(); ++i) {
      if (!members.at(i)->is_same_as(*other2.members.at(i))) {
        return false;
      }
    }
    return true;
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "Struct<";
    bool first = true;
    for (const auto& member : members) {
      if (first) {
        first = false;
      } else {
        s << ",";
      }
      s << *member;
    }
    s << ">";
  }
};
struct TypePointer : public Type {
  std::shared_ptr<Type> inner;
  TypePointer(const std::shared_ptr<Type>& inner) : Type(L_TYPE_CLASS_POINTER),
    inner(inner) {}
  virtual bool is_same_as(const Type& other) const override final {
    if (other.cls != cls) { return false; }
    const auto& other2 = (const TypePointer&)other;
    return is_same_as(*other2.inner);
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "Pointer<" << *inner << ">";
  }
};



struct MemoryFunctionVariable : public Memory {
  const void* handle;

  inline MemoryFunctionVariable(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    const void* handle // Used to identify same-source local variable accesses.
  ) : Memory(L_MEMORY_CLASS_FUNCTION_VARIABLE, ty,
    std::forward<AccessChain>(ac)), handle(handle) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "$" << s.get_var_name_by_handle(handle) << ":" << *ty;
  }
};

struct MemoryDescriptor : public Memory {
  uint32_t binding;
  uint32_t set;

protected:
  inline MemoryDescriptor(
    MemoryClass cls,
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : Memory(cls, ty, std::forward<AccessChain>(ac)),
    binding(binding), set(set) {}
};

struct MemoryUniformBuffer : public MemoryDescriptor {
  inline MemoryUniformBuffer(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : MemoryDescriptor(L_MEMORY_CLASS_UNIFORM_BUFFER, ty,
    std::forward<AccessChain>(ac), binding, set) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "UniformBuffer@" << binding << "," << set << "[" << ac << "]:" << *ty;
  }
};
struct MemoryStorageBuffer : public MemoryDescriptor {
  size_t offset;
  inline MemoryStorageBuffer(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : MemoryDescriptor(L_MEMORY_CLASS_STORAGE_BUFFER, ty,
    std::forward<AccessChain>(ac), binding, set) {}
};
struct MemorySampledImage : public MemoryDescriptor {
  inline MemorySampledImage(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : MemoryDescriptor(L_MEMORY_CLASS_SAMPLED_IMAGE, ty,
    std::forward<AccessChain>(ac), binding, set) {}
};
struct MemorStorageImage : public MemoryDescriptor {
  inline MemorStorageImage(
    const std::shared_ptr<Type>& ty,
    AccessChain&& ac,
    uint32_t binding,
    uint32_t set
  ) : MemoryDescriptor(L_MEMORY_CLASS_STORAGE_IMAGE, ty,
    std::forward<AccessChain>(ac), binding, set) {}
};



struct ExprConstant : public Expr {
  const std::vector<uint32_t> lits;

  inline ExprConstant(
    const std::shared_ptr<Type>& ty,
    std::vector<uint32_t>&& lits
  ) : Expr(L_EXPR_OP_CONSTANT, ty), lits(lits) {}

  virtual void dbg_print(Debug& s) const override final {
    if (ty->cls == L_TYPE_CLASS_INT) {
      const auto& ty2 = *(const TypeInt*)ty.get();
      if (ty2.is_signed) {
        if (ty2.nbit == 32) {
          s << *(const int32_t*)&lits[0];
        } else {
          liong::unimplemented();
        }
      } else {
        if (ty2.nbit == 32) {
          s << *(const uint32_t*)&lits[0];
        } else {
          liong::unimplemented();
        }
      }
    } else if (ty->cls == L_TYPE_CLASS_FLOAT) {
      const auto& ty2 = *(const TypeFloat*)ty.get();
      if (ty2.nbit == 32) {
        s << *(const float*)&lits[0];
      } else {
        liong::unimplemented();
      }
    } else if (ty->cls == L_TYPE_CLASS_BOOL) {
      s << (lits[0] != 0 ? "true" : "false");
    }
    s << ":" << *ty;
  }
};

struct ExprLoad : public Expr {
  const std::shared_ptr<Memory> src_ptr;

  inline ExprLoad(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Memory>& src_ptr
  ) : Expr(L_EXPR_OP_LOAD, ty), src_ptr(src_ptr) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "Load(" << *src_ptr << ")";
  }
};

struct ExprBinaryOp : public Expr {
  const std::shared_ptr<Expr> a;
  const std::shared_ptr<Expr> b;

  inline ExprBinaryOp(
    ExprOp op,
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : Expr(op, ty), a(a), b(b) {
    liong::assert(a && b && a->ty->is_same_as(*b->ty));
  }
};

struct ExprAdd : public ExprBinaryOp {
  inline ExprAdd(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(L_EXPR_OP_ADD, ty, a, b) {
    liong::assert(ty->is_same_as(*a->ty));
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "(" << *a << " + " << *b << ")";
  }
};
struct ExprSub : public ExprBinaryOp {
  inline ExprSub(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(L_EXPR_OP_SUB, ty, a, b) {
    liong::assert(ty->is_same_as(*a->ty));
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "(" << *a << " - " << *b << ")";
  }
};

struct ExprLt : public ExprBinaryOp {
  inline ExprLt(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(L_EXPR_OP_LT, ty, a, b) {
    liong::assert(ty->cls == L_TYPE_CLASS_BOOL);
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "(" << *a << " < " << *b << ")";
  }
};
struct ExprEq : public ExprBinaryOp {
  inline ExprEq(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(L_EXPR_OP_EQ, ty, a, b) {
    liong::assert(ty->cls == L_TYPE_CLASS_BOOL);
  }

  virtual void dbg_print(Debug& s) const override final {
    s << "(" << *a << " < " << *b << ")";
  }
};
struct ExprTypeCast : public Expr {
  std::shared_ptr<Expr> src;
  inline ExprTypeCast(
    const std::shared_ptr<Type>& dst_ty,
    const std::shared_ptr<Expr>& src
  ) : Expr(L_EXPR_OP_TYPE_CAST, dst_ty), src(src) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "(" << *src << ":" << *ty << ")";
  }
};



struct StmtStore : public Stmt {
  const std::shared_ptr<Memory> dst_ptr;
  const std::shared_ptr<Expr> value;

  inline StmtStore(
    const std::shared_ptr<Memory>& dst_ptr,
    const std::shared_ptr<Expr>& value
  ) : Stmt(L_STMT_OP_STORE), dst_ptr(dst_ptr), value(value) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "Store(" << *dst_ptr << ", " << *value << ")";
  }
};



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
struct ControlFlowBackEdge: public ControlFlow {
  inline ControlFlowBackEdge(
    const InstructionRef& label
  ) : ControlFlow(L_CONTROL_FLOW_CLASS_BACK_EDGE, label, {}) {}

  virtual void dbg_print(Debug& s) const override final {
    s << "continue" << std::endl;
  }
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

extern std::map<std::string, std::unique_ptr<ControlFlow>> extract_entry_points(const SpirvModule& mod);
