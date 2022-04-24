// Visitor-based utilities.
// @PENGUINLIONG
#pragma once
#include "node/node.hpp"

extern std::string dbg_print(const NodeRef<Node>& x);
extern bool is_tail_stmt(const StmtRef& x);
extern StmtRef flatten_block(const StmtRef& x);
