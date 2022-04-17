// SPIR-V abstract.
// @PENGUINLIONG
#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include "spv/instr.hpp"

struct SpirvHeader {
  uint32_t magic;
  uint32_t version;
  uint32_t generator_magic;
  uint32_t bound;
  uint32_t reserved;
};
struct SpirvAbstract {
  SpirvHeader head;
  std::map<spv::Id, InstructionRef> id2instr_map;
  const uint32_t* beg;
  const uint32_t* end;

  const InstructionRef& lookup_instr(spv::Id id) const {
    return id2instr_map.at(id);
  }
};

SpirvAbstract scan_spirv(const std::vector<uint32_t>& spv);
