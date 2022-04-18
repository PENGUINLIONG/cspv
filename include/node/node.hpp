// Abstraction of everything in the control flow graph.
// @PENGUINLIONG
#pragma once
#include <memory>
#include <vector>
#include "gft/assert.hpp"

enum NodeVariant {
    L_NODE_VARIANT_MEMORY,
    L_NODE_VARIANT_TYPE,
    L_NODE_VARIANT_EXPR,
    L_NODE_VARIANT_STMT,
};
struct Node {
    const NodeVariant nova;
    inline Node(NodeVariant nova) : nova(nova) {}
};

typedef std::shared_ptr<Node> NodeRef;

struct Memory;
struct Type;
struct Expr;
struct Stmt;
