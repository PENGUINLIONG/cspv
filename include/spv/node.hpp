// Abstraction of everything in the control flow graph.
// @PENGUINLIONG
#pragma once

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
