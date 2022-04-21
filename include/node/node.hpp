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

template<typename T = Node>
struct NodeRef {
  NodeRef() : alloc(nullptr), ref(nullptr) {}
  NodeRef(T* ptr) : alloc(std::shared_ptr<Node>(ptr)), ref(ptr) {}
  NodeRef(const NodeRef<T>& b) : alloc(b.alloc), ref(b.ref) {}
  NodeRef(NodeRef<T>&& b) : alloc(std::exchange(b.alloc, nullptr)), ref(std::exchange(b.ref, nullptr)) {}
  NodeRef(std::shared_ptr<Node>&& alloc, T* ref) :
    alloc(std::forward<std::shared_ptr<Node>>(alloc)), ref(ref) {}

  inline NodeRef<T>& operator=(const NodeRef<T>& b) {
    alloc = b.alloc;
    ref = b.ref;
    return *this;
  }
  inline NodeRef<T>& operator=(NodeRef<T>&& b) {
    alloc = std::exchange(b.alloc, nullptr);
    ref = std::exchange(b.ref, nullptr);
    return *this;
  }

  constexpr T* get() { return ref; }
  constexpr const T* get() const { return ref; }

  constexpr Node* get_alloc() { return alloc.get(); }
  constexpr const Node* get_alloc() const { return alloc.get(); }

  template<typename U>
  inline NodeRef<U> as() const {
    return NodeRef<U>(std::shared_ptr<Node>(alloc), (U*)ref);
  }

  constexpr T* operator->() { return ref; }
  constexpr const T* operator->() const { return ref; }
  constexpr T& operator*() { return *ref; }
  constexpr const T& operator*() const { return *ref; }

  constexpr bool operator==(nullptr_t) const { return ref == nullptr; }
  constexpr bool operator!=(nullptr_t) const { return ref != nullptr; }

  template<typename U>
  constexpr bool operator==(const NodeRef<U>& b) const { return get_alloc() == b.get_alloc(); }
  template<typename U>
  constexpr bool operator!=(const NodeRef<U>& b) const { return get_alloc() != b.get_alloc(); }
  template<typename U>
  constexpr bool operator<(const NodeRef<U>& b) const { return get_alloc() < b.get_alloc(); }

private:
  std::shared_ptr<Node> alloc;
  T* ref;
};

struct Memory;
struct Type;
struct Expr;
struct Stmt;
