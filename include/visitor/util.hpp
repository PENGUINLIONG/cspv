// Visitor-based utilities.
// @PENGUINLIONG
#pragma once
#include "node/node.hpp"

extern std::string dbg_print(const NodeRef& x);
extern bool is_tail_stmt(const StmtRef& x);
extern StmtRef flatten_block(const StmtRef& x);
extern StmtRef& get_head_stmt(StmtRef& stmt);
extern StmtRef& get_tail_stmt(StmtRef& stmt);
extern std::vector<NodeRef> collect_children(const NodeRef& node);
extern bool match_pattern(const NodeRef& pattern, const NodeRef& target);

enum PredefinedType {
  L_PREDEFINED_TYPE_UNDEFINED,
  L_PREDEFINED_TYPE_BOOL,
  L_PREDEFINED_TYPE_U32,
  L_PREDEFINED_TYPE_I32,
  L_PREDEFINED_TYPE_F32,
  L_PREDEFINED_TYPE_U64,
  L_PREDEFINED_TYPE_I64,
  L_PREDEFINED_TYPE_F64,
};
extern PredefinedType get_predefined_ty(const TypeRef& ty);
