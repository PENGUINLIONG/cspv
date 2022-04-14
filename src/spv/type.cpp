#include "spv/ast.hpp"

using namespace liong;

std::shared_ptr<Type> parse_ty(const SpirvModule& mod, const InstructionRef& instr) {
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
  } else if (op == spv::Op::OpTypeStruct) {
    auto e = instr.extract_params();
    std::vector<std::shared_ptr<Type>> members;
    while (e) {
      members.emplace_back(parse_ty(mod, e.read_id()));
    }
    return std::shared_ptr<Type>(new TypeStruct(std::move(members)));
  } else if (op == spv::Op::OpTypePointer) {
    auto e = instr.extract_params();
    spv::StorageClass storage_cls = e.read_u32_as<spv::StorageClass>();
    auto inner = parse_ty(mod, e.read_id());
    return std::shared_ptr<Type>(new TypePointer(inner));
  } else {
    panic("unsupported type");
  }
}
std::shared_ptr<Type> parse_ty(const SpirvModule& mod, spv::Id id) {
  return parse_ty(mod, mod.lookup_instr(id));
}
