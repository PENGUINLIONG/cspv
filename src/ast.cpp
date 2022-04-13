#include "ast.hpp"

using namespace liong;

struct TypeVoid : public Type {
  TypeVoid() : Type(L_TYPE_CLASS_VOID) {}
  virtual bool is_same_as(const Type& other) const override final {
    return other.cls == cls;
  }
};
struct TypeBool : public Type {
  TypeBool() : Type(L_TYPE_CLASS_BOOL) {}
  virtual bool is_same_as(const Type& other) const override final {
    return other.cls == cls;
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
};
struct TypeFloat : public Type {
  uint32_t nbit;
  TypeFloat(uint32_t nbit) : Type(L_TYPE_CLASS_FLOAT), nbit(nbit) {}
  virtual bool is_same_as(const Type& other) const override final {
    if (other.cls != cls) { return false; }
    const auto& other2 = (const TypeFloat&)other;
    return other2.nbit == nbit;
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
};


std::shared_ptr<Type> parse_ty(const SpirvAbstract& abstr, spv::Id id);
std::shared_ptr<Type> parse_ty(
  const SpirvAbstract& abstr,
  const InstructionRef& instr
) {
  spv::Op op = instr.op();
  if (op == spv::Op::OpTypeVoid) {
    return std::shared_ptr<Type>(new TypeVoid);
  } else if (op == spv::Op::OpTypeBool) {
    return std::shared_ptr<Type>(new TypeBool);
  } else if (op == spv::Op::OpTypeInt) {
    auto e = instr.extract_params();
    uint32_t nbit = e.read_u32();
    bool is_signed = e.read_bool();
    return std::shared_ptr<Type>(new TypeInt(nbit, is_signed));
  } else if (op == spv::Op::OpTypeFloat) {
    auto e = instr.extract_params();
    uint32_t nbit = e.read_u32();
    return std::shared_ptr<Type>(new TypeFloat(nbit));
  } else if (op == spv::Op::OpTypePointer) {
    auto e = instr.extract_params();
    spv::StorageClass storage_cls = e.read_u32_as<spv::StorageClass>();
    auto inner = parse_ty(abstr, abstr.id2instr_map.at(e.read_id()));
    return std::shared_ptr<Type>(new TypePointer(inner));
  } else {
    panic("unsupported type");
  }
}
std::shared_ptr<Type> parse_ty(const SpirvAbstract& abstr, spv::Id id) {
  return parse_ty(abstr, abstr.id2instr_map.at(id));
}


struct ExprConstant : public Expr {
  const std::vector<uint32_t> lits;

  inline ExprConstant(
    const std::shared_ptr<Type>& ty,
    std::vector<uint32_t>&& lits
  ) : Expr(L_EXPR_OP_CONSTANT, ty), lits(lits) {}
};
struct ExprVariable : public Expr {
  spv::StorageClass store_cls;

    inline ExprVariable(
    const std::shared_ptr<Type>& ty,
    spv::StorageClass store_cls
  ) : Expr(L_EXPR_OP_VARIABLE, ty), store_cls(store_cls) {}
};

struct ExprLoad : public Expr {
  const std::shared_ptr<Expr> src_ptr;

  inline ExprLoad(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& src_ptr
  ) : Expr(L_EXPR_OP_LOAD, ty), src_ptr(src_ptr) {}
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
    assert(a && b && a->ty->is_same_as(*b->ty));
  }
};

struct ExprAdd : public ExprBinaryOp {
  inline ExprAdd(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(L_EXPR_OP_ADD, ty, a, b) {
    assert(ty->is_same_as(*a->ty));
  }
};
struct ExprSub : public ExprBinaryOp {
  inline ExprSub(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(L_EXPR_OP_SUB, ty, a, b) {
    assert(ty->is_same_as(*a->ty));
  }
};

struct ExprLt : public ExprBinaryOp {
  inline ExprLt(
    const std::shared_ptr<Type>& ty,
    const std::shared_ptr<Expr>& a,
    const std::shared_ptr<Expr>& b
  ) : ExprBinaryOp(L_EXPR_OP_LT, ty, a, b) {
    assert(ty->cls == L_TYPE_CLASS_BOOL);
  }
};

std::shared_ptr<Expr> parse_expr(const SpirvAbstract& abstr, spv::Id id);
std::shared_ptr<Expr> parse_expr(
  const SpirvAbstract& abstr,
  const InstructionRef& instr
) {
  std::shared_ptr<Expr> out;
  spv::Op op = instr.op();
  if (op == spv::Op::OpConstant) {
    auto e = instr.extract_params();
    std::vector<uint32_t> lits;
    auto ty = parse_ty(abstr, instr.result_ty_id());
    while (e) {
      lits.emplace_back(e.read_u32());
    }
    out = std::shared_ptr<Expr>(new ExprConstant(ty, std::move(lits)));
  } else if (op == spv::Op::OpVariable) {
    auto ty = parse_ty(abstr, instr.result_ty_id());
    auto e = instr.extract_params();
    auto store_cls = e.read_u32_as<spv::StorageClass>();
    out = std::shared_ptr<Expr>(new ExprVariable(ty, store_cls));
  } else if (op == spv::Op::OpLoad) {
    auto ty = parse_ty(abstr, instr.result_ty_id());
    auto e = instr.extract_params();
    auto src_ptr = parse_expr(abstr, e.read_id());
    out = std::shared_ptr<Expr>(new ExprLoad(ty, src_ptr));
  } else if (op == spv::Op::OpIAdd) {
    auto ty = parse_ty(abstr, instr.result_ty_id());
    auto e = instr.extract_params();
    auto a = parse_expr(abstr, e.read_id());
    auto b = parse_expr(abstr, e.read_id());
    out = std::shared_ptr<Expr>(new ExprAdd(ty, a, b));
  } else if (op == spv::Op::OpSLessThan) {
    auto ty = parse_ty(abstr, instr.result_ty_id());
    auto e = instr.extract_params();
    auto a = parse_expr(abstr, e.read_id());
    auto b = parse_expr(abstr, e.read_id());
    out = std::shared_ptr<Expr>(new ExprLt(ty, a, b));
  }
  return out;
}
std::shared_ptr<Expr> parse_expr(const SpirvAbstract& abstr, spv::Id id) {
  return parse_expr(abstr, abstr.id2instr_map.at(id));
}



struct StmtStore : public Stmt {
  const std::shared_ptr<Expr> dst_ptr;
  const std::shared_ptr<Expr> value;

  inline StmtStore(
    const std::shared_ptr<Expr>& dst_ptr,
    const std::shared_ptr<Expr>& value
  ) : Stmt(L_STMT_OP_STORE), dst_ptr(dst_ptr), value(value) {}
};


std::shared_ptr<Stmt> parse_stmt(
  const SpirvAbstract& abstr,
  const InstructionRef& instr
) {
  std::shared_ptr<Stmt> out;
  spv::Op op = instr.op();
  if (op == spv::Op::OpStore) {
    auto e = instr.extract_params();
    auto dst_ptr = parse_expr(abstr, e.read_id());
    auto value = parse_expr(abstr, e.read_id());
    assert(dst_ptr != nullptr);
    assert(value != nullptr);
    out = (std::shared_ptr<Stmt>)(Stmt*)new StmtStore { dst_ptr, value };
  }
  return out;
}
