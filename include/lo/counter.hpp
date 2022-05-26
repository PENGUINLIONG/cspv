// IR class counter.
// @PENGUINLIONG
#include <cstdint>

template<typename T>
struct Counter {
  static const uint32_t INVALID_CLASS = 0;
  static uint32_t n = 0; // 1-based.

  inline static uint32_t& get_id() { return ++n; }
};
