// AST Infrastructure.
// @PENGUINLIONG
#pragma once
#include <memory>
#include <ostream>
#include <utility>
#include "gft/assert.hpp"
#include "spv/mod.hpp"
#include "spv/type.hpp"
#include "spv/memory.hpp"
#include "spv/expr.hpp"
#include "spv/stmt.hpp"
#include "spv/ctrl-flow.hpp"

extern std::map<std::string, std::unique_ptr<ControlFlow>> extract_entry_points(const SpirvModule& mod);
