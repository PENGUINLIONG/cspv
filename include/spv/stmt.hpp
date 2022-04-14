// Statements op implementations.
// @PENGUINLIONG
#pragma once
#include "spv/stmt-reg.hpp"

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

std::shared_ptr<Stmt> parse_stmt(const SpirvModule& mod, const InstructionRef& ptr);
std::shared_ptr<Stmt> parse_stmt(const SpirvModule& mod, spv::Id id);
