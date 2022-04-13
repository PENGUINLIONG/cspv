// AST Infrastructure.
// @PENGUINLIONG
#pragma once
#include <memory>
#include <ostream>
#include <utility>
#include "gft/assert.hpp"
#include "spv/mod.hpp"

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

  virtual void dbg_print(std::ostream& s) const { s << "type?"; }

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

  L_EXPR_OP_AND,
  L_EXPR_OP_OR,
  L_EXPR_OP_NOT,
  L_EXPR_OP_XOR,
};
struct Expr {
  const ExprOp op;
  const std::shared_ptr<Type> ty;

  virtual bool is_same_as(const Type& other) const {
    liong::unimplemented();
  }

  virtual void dbg_print(std::ostream& s) const { s << "expr?"; }

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

  virtual void dbg_print(std::ostream& s) const { s << "memory?"; }

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

  virtual void dbg_print(std::ostream& s) const { s << "stmt?"; }

protected:
  inline Stmt(StmtOp op) : op(op) {}
};



inline std::ostream& operator<<(std::ostream& s, const Type& x) {
  x.dbg_print(s);
  return s;
}
inline std::ostream& operator<<(std::ostream& s, const Expr& x) {
  x.dbg_print(s);
  return s;
}
inline std::ostream& operator<<(std::ostream& s, const AccessChain& x) {
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
inline std::ostream& operator<<(std::ostream& s, const Memory& x) {
  x.dbg_print(s);
  return s;
}
inline std::ostream& operator<<(std::ostream& s, const Stmt& x) {
  x.dbg_print(s);
  return s;
}








struct TypeVoid : public Type {
  TypeVoid() : Type(L_TYPE_CLASS_VOID) {}
  virtual bool is_same_as(const Type& other) const override final {
    return other.cls == cls;
  }

  virtual void dbg_print(std::ostream& s) const override final {
    s << "void";
  }
};
struct TypeBool : public Type {
  TypeBool() : Type(L_TYPE_CLASS_BOOL) {}
  virtual bool is_same_as(const Type& other) const override final {
    return other.cls == cls;
  }

  virtual void dbg_print(std::ostream& s) const override final {
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

  virtual void dbg_print(std::ostream& s) const override final {
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

  virtual void dbg_print(std::ostream& s) const override final {
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

  virtual void dbg_print(std::ostream& s) const override final {
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

  virtual void dbg_print(std::ostream& s) const override final {
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

  virtual void dbg_print(std::ostream& s) const override final {
    s << "$" << handle << ":" << *ty;
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

  virtual void dbg_print(std::ostream& s) const override final {
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

  virtual void dbg_print(std::ostream& s) const override final {
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

  virtual void dbg_print(std::ostream& s) const override final {
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

  virtual void dbg_print(std::ostream& s) const override final {
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

  virtual void dbg_print(std::ostream& s) const override final {
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

  virtual void dbg_print(std::ostream& s) const override final {
    s << "(" << *a << " < " << *b<< ")";
  }
};

struct StmtStore : public Stmt {
  const std::shared_ptr<Memory> dst_ptr;
  const std::shared_ptr<Expr> value;

  inline StmtStore(
    const std::shared_ptr<Memory>& dst_ptr,
    const std::shared_ptr<Expr>& value
  ) : Stmt(L_STMT_OP_STORE), dst_ptr(dst_ptr), value(value) {}

  virtual void dbg_print(std::ostream& s) const override final {
    s << "Store(" << *dst_ptr << ", " << *value << ")";
  }
};



struct ControlFlow;

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
};

struct ControlFlowSelection {
  std::vector<Branch> branches;
};
struct ControlFlowInitLoop {
  std::unique_ptr<ControlFlow> body;
};
struct ControlFlowLoop {
  std::shared_ptr<Expr> cond;
  std::unique_ptr<ControlFlow> body;
};
struct ControlFlowReturn {
  InstructionRef rv;
};
struct ControlFlow {
  InstructionRef label;
  std::unique_ptr<ControlFlow> next;
  std::vector<std::shared_ptr<Stmt>> stmts;
  // Extra parameters.
  std::unique_ptr<ControlFlowSelection> sel;
  std::unique_ptr<ControlFlowReturn> ret;
  std::unique_ptr<ControlFlowLoop> loop;
};

extern std::map<std::string, std::unique_ptr<ControlFlow>> extract_entry_points(const SpirvModule& mod);