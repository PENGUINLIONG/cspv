// AST Infrastructure.
// @PENGUINLIONG
#pragma once
#include <map>
#include <memory>
#include "spv/mod.hpp"

std::map<std::string, StmtRef> extract_entry_points(SpirvModule& mod);
