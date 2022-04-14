#include "spv/ast.hpp"

using namespace liong;

std::shared_ptr<Stmt> parse_stmt(const SpirvModule& mod, const InstructionRef& instr) {
  std::shared_ptr<Stmt> out;
  spv::Op op = instr.op();
  if (op == spv::Op::OpStore) {
    auto e = instr.extract_params();
    auto dst_ptr = parse_mem(mod, e.read_id());
    auto value = parse_expr(mod, e.read_id());
    assert(dst_ptr != nullptr);
    assert(value != nullptr);
    out = (std::shared_ptr<Stmt>)(Stmt*)new StmtStore(dst_ptr, value);
  }
  return out;
}
std::shared_ptr<Stmt> parse_stmt(const SpirvModule& mod, spv::Id id) {
  return parse_stmt(mod, mod.lookup_instr(id));
}
