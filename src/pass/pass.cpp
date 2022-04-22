#include <memory>
#include <map>
#include <vector>
#include <string>
#include "pass/pass.hpp"

using namespace liong;

struct PassRegistry {
  std::map<std::string, std::unique_ptr<Pass>> inner;
};
std::unique_ptr<PassRegistry> PASS_REG;

Pass* reg_pass(std::unique_ptr<Pass>&& pass) {
  if (PASS_REG == nullptr) {
    PASS_REG = std::make_unique<PassRegistry>();
  }
  return (*PASS_REG).inner
    .emplace(pass->name, std::forward<std::unique_ptr<Pass>>(pass))
    .first->second.get();
}
void apply_pass(const std::string& name, NodeRef<Node>& node) {
  auto it = PASS_REG->inner.find(name);
  assert(it != PASS_REG->inner.end(), "'", name, "' is not a registered pass");
  it->second->apply(node);
}
