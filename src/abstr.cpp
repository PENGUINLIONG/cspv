#include "gft/assert.hpp"
#include "spv/abstr.hpp"

using namespace liong;

SpirvAbstract scan_spirv(const std::vector<uint32_t>& spv) {
  SpirvAbstract out {};
  out.head.magic = spv[0];
  out.head.version = spv[1];
  out.head.generator_magic = spv[2];
  out.head.bound = spv[3];
  out.head.reserved = spv[4];

  out.beg = spv.data() + 5;
  out.end = spv.data() + spv.size();
  InstructionRef cur = out.beg;
  while (cur < out.end) {
    const InstructionRef instr(cur);
    spv::Op op = instr.op();

    // Ignore source line debug info.
    if (op == spv::Op::OpLine || op == spv::Op::OpNoLine) {
      goto done;
    }

    spv::Id result_id = instr.result_id();
    if (result_id) {
      auto it = out.id2instr_map.find(result_id);
      if (it == out.id2instr_map.end()) {
        out.id2instr_map.emplace(std::make_pair(result_id, cur));
      } else {
        panic("result id #", result_id, " is assigned by more than one "
          "instructions");
      }
    }

  done:
    cur = cur.next();
  }

  return out;
}
