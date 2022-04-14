#include "spv/ast.hpp"

using namespace liong;

std::shared_ptr<Expr> parse_expr(const SpirvModule& mod, const InstructionRef& instr) {
  std::shared_ptr<Expr> out;
  spv::Op op = instr.op();
  switch (op) {
  case spv::Op::OpConstant:
  {
    auto e = instr.extract_params();
    std::vector<uint32_t> lits;
    auto ty = parse_ty(mod, instr.result_ty_id());
    while (e) {
      lits.emplace_back(e.read_u32());
    }
    out = std::shared_ptr<Expr>(new ExprConstant(ty, std::move(lits)));
    break;
  }
  case spv::Op::OpConstantTrue:
  {
    auto ty = parse_ty(mod, instr.result_ty_id());
    std::vector<uint32_t> lits;
    lits.emplace_back(1);
    out = std::shared_ptr<Expr>(new ExprConstant(ty, std::move(lits)));
    break;
  }
  case spv::Op::OpConstantFalse:
  {
    auto ty = parse_ty(mod, instr.result_ty_id());
    std::vector<uint32_t> lits;
    lits.emplace_back(0);
    out = std::shared_ptr<Expr>(new ExprConstant(ty, std::move(lits)));
    break;
  }
  case spv::Op::OpLoad:
  {
    auto ty = parse_ty(mod, instr.result_ty_id());
    auto e = instr.extract_params();
    auto src_ptr = parse_mem(mod, e.read_id());
    out = std::shared_ptr<Expr>(new ExprLoad(ty, src_ptr));
    break;
  }
  case spv::Op::OpIAdd:
  case spv::Op::OpFAdd:
  {
    auto ty = parse_ty(mod, instr.result_ty_id());
    auto e = instr.extract_params();
    auto a = parse_expr(mod, e.read_id());
    auto b = parse_expr(mod, e.read_id());
    out = std::shared_ptr<Expr>(new ExprAdd(ty, a, b));
    break;
  }
  case spv::Op::OpSLessThan:
  case spv::Op::OpULessThan:
  case spv::Op::OpFOrdLessThan:
  {
    auto ty = parse_ty(mod, instr.result_ty_id());
    auto e = instr.extract_params();
    auto a = parse_expr(mod, e.read_id());
    auto b = parse_expr(mod, e.read_id());
    out = std::shared_ptr<Expr>(new ExprLt(ty, a, b));
    break;
  }
  case spv::Op::OpConvertFToS:
  case spv::Op::OpConvertSToF:
  case spv::Op::OpConvertFToU:
  case spv::Op::OpConvertUToF:
  {
    auto dst_ty = parse_ty(mod, instr.result_ty_id());
    auto e = instr.extract_params();
    auto src = parse_expr(mod, e.read_id());
    out = std::shared_ptr<Expr>(new ExprTypeCast(dst_ty, src));
    break;
  }
  case spv::Op::OpIEqual:
  case spv::Op::OpFOrdEqual:
  {
    auto ty = parse_ty(mod, instr.result_ty_id());
    auto e = instr.extract_params();
    auto a = parse_expr(mod, e.read_id());
    auto b = parse_expr(mod, e.read_id());
    out = std::shared_ptr<Expr>(new ExprEq(ty, a, b));
    break;
  }
  default:
    panic("unexpected expr op (", uint32_t(op), ")");
  }
  return out;
}
std::shared_ptr<Expr> parse_expr(const SpirvModule& mod, spv::Id id) {
  return parse_expr(mod, mod.lookup_instr(id));
}
