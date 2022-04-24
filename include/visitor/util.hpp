// Visitor-based utilities.
// @PENGUINLIONG
#pragma once
#include "node/node.hpp"

extern std::string dbg_print(const NodeRef<Node>& x);
extern bool is_tail_stmt(const StmtRef& x);
extern StmtRef flatten_block(const StmtRef& x);
extern StmtRef& get_head_stmt(StmtRef& stmt);
extern StmtRef& get_tail_stmt(StmtRef& stmt);
extern std::vector<NodeRef<Node>> collect_children(const NodeRef<Node>& node);
extern bool match_pattern(const NodeRef<Node>& pattern, const NodeRef<Node>& target);
