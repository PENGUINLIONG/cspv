#include "spv/ast.hpp"

using namespace liong;

std::shared_ptr<Stmt> parse_stmt(const SpirvAbstract& abstr, spv::Id id) {
  return parse_stmt(abstr, abstr.lookup_instr(id));
}
