// SPIR-V syntactic segmentation and parsing.
// @PENGUINLIONG
#pragma once
#include "gft/log.hpp"
#include "gft/assert.hpp"
#include "gft/util.hpp"
#define SPV_ENABLE_UTILITY_CODE
#include "spirv/unified1/spirv.hpp11"

const spv::Id L_INVALID_ID = spv::Id(0);

struct InstructionParameterExtractor {
  const uint32_t* cur;
  const uint32_t* end;

  inline InstructionParameterExtractor(
    const uint32_t* cur,
    const uint32_t* end
  ) : cur(cur), end(end) {}

  operator bool() const {
    return cur < end;
  }

  inline uint32_t read_u32() {
    liong::assert(cur < end);
    return *(cur++);
  }
  inline bool read_bool() {
    return read_u32() != 0;
  }
  inline spv::Id read_id() {
    return spv::Id(read_u32());
  }
  inline const char* read_str() {
    const char* out = (const char*)cur;
    size_t size = std::strlen((const char*)cur) + 1;
    cur += liong::util::div_up(size, sizeof(uint32_t));
    return out;
  }
  template<typename T>
  inline T read_u32_as() {
    return T(read_u32());
  }
};
struct InstructionRef {
  spv::Op op_;
  uint32_t len_;
  const uint32_t* inner;

  inline InstructionRef() : inner(nullptr), op_(spv::Op::OpNop), len_(0) {}
  inline InstructionRef(const uint32_t* inner) :
    inner(inner),
    op_(inner != nullptr ? (spv::Op)(*inner & 0xffff) : spv::Op::OpNop),
    len_(inner != nullptr ? (*inner >> 16) : 0) {}
  inline InstructionRef(const InstructionRef& rhs) :
    inner(rhs.inner),
    op_(rhs.op_),
    len_(rhs.len_) {}
  inline InstructionRef(InstructionRef&& rhs) :
    inner(std::exchange(rhs.inner, nullptr)),
    op_(std::exchange(rhs.op_, spv::Op::OpNop)),
    len_(std::exchange(rhs.len_, (uint32_t)0)) {}

  inline InstructionRef& operator=(const InstructionRef& rhs) {
    inner = rhs.inner;
    op_ = rhs.op_;
    len_ = rhs.len_;
    return *this;
  }
  inline InstructionRef& operator=(InstructionRef&& rhs) {
    inner = std::exchange(rhs.inner, nullptr);
    op_ = std::exchange(rhs.op_, spv::Op::OpNop);
    len_ = std::exchange(rhs.len_, (uint32_t)0);
    return *this;
  }

  inline InstructionRef next() {
    return inner + len();
  }

  constexpr bool operator==(std::nullptr_t) const { return inner == nullptr; }
  constexpr bool operator!=(std::nullptr_t) const { return inner != nullptr; }
  constexpr bool operator==(const InstructionRef& rhs) const {
    return inner == rhs.inner;
  }
  constexpr bool operator!=(const InstructionRef& rhs) const {
    return inner != rhs.inner;
  }
  constexpr bool operator<(const InstructionRef& rhs) const {
    return inner < rhs.inner;
  }
  constexpr bool operator>=(const InstructionRef& rhs) const {
    return inner >= rhs.inner;
  }

  constexpr operator bool() const {
    return *this != nullptr;
  }

  constexpr const uint32_t* words() const {
    return inner;
  }

  constexpr spv::Op op() const {
    return op_;
  }
  constexpr size_t len() const {
    return len_;
  }

  inline spv::Id result_ty_id() const {
    bool has_result_ty_id, has_result_id;
    spv::HasResultAndType(op(), &has_result_id, &has_result_ty_id);

    if (has_result_ty_id) {
      return inner[1];
    } else {
      return L_INVALID_ID;
    }
  }
  inline spv::Id result_id() const {
    bool has_result_ty_id, has_result_id;
    spv::HasResultAndType(op(), &has_result_id, &has_result_ty_id);

    if (has_result_id) {
      return has_result_ty_id ? inner[2] : inner[1];
    } else {
      return L_INVALID_ID;
    }
  }

  inline InstructionParameterExtractor extract_params() const {
    bool has_result_ty_id, has_result_id;
    spv::HasResultAndType(op(), &has_result_id, &has_result_ty_id);

    const uint32_t* param_beg;
    if (has_result_id) {
      param_beg = has_result_ty_id ? inner + 3 : inner + 2;
    } else {
      param_beg = inner + 1;
    }
    const uint32_t* param_end = inner + len();

    return InstructionParameterExtractor(param_beg, param_end);
  }
};
