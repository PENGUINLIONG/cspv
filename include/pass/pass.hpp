// Pass: Ranged-loop elevation
// @PENGUINLIONG
#include "visitor/visitor.hpp"

struct Pass {
  const std::string name;
  Pass(const std::string& name) : name(name) {}

  virtual void apply(NodeRef& node) {}
};

Pass* reg_pass(std::unique_ptr<Pass>&& pass);
template<typename T>
inline Pass* reg_pass() {
  return reg_pass(std::make_unique<T>());
}
void apply_pass(const std::string& name, NodeRef& node);
