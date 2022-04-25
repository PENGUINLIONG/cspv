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

  virtual void collect_children(struct NodeDrain* drain) const {}
};

template<typename T>
struct Reference {
  Reference() : alloc(nullptr), ref(nullptr) {}
  Reference(T* ptr) : alloc(std::shared_ptr<Node>(ptr)), ref(ptr) {}
  Reference(const Reference<T>& b) : alloc(b.alloc), ref(b.ref) {}
  Reference(Reference<T>&& b) :
    alloc(std::exchange(b.alloc, nullptr)),
    ref(std::exchange(b.ref, nullptr)) {}
  Reference(std::shared_ptr<Node>&& alloc, T* ref) :
    alloc(std::forward<std::shared_ptr<Node>>(alloc)), ref(ref) {}

  inline Reference<T>& operator=(const Reference<T>& b) {
    alloc = b.alloc;
    ref = b.ref;
    return *this;
  }
  inline Reference<T>& operator=(Reference<T>&& b) {
    alloc = std::exchange(b.alloc, nullptr);
    ref = std::exchange(b.ref, nullptr);
    return *this;
  }

  constexpr T* get() { return ref; }
  constexpr const T* get() const { return ref; }

  constexpr Node* get_alloc() { return alloc.get(); }
  constexpr const Node* get_alloc() const { return alloc.get(); }

  template<typename U>
  inline Reference<U> as() const {
    return Reference<U>(std::shared_ptr<Node>(alloc), (U*)ref);
  }

  constexpr T* operator->() { return ref; }
  constexpr const T* operator->() const { return ref; }
  constexpr T& operator*() { return *ref; }
  constexpr const T& operator*() const { return *ref; }

  constexpr bool operator==(nullptr_t) const { return ref == nullptr; }
  constexpr bool operator!=(nullptr_t) const { return ref != nullptr; }

  template<typename U,
    typename _ = std::enable_if_t<std::is_base_of_v<T, U> || std::is_base_of_v<U, T>>>
  inline operator Reference<U>() {
    return Reference<U>(std::shared_ptr<Node>(alloc), (U*)ref);
  }

  template<typename U>
  constexpr bool operator==(const Reference<U>& b) const {
    return get_alloc() == b.get_alloc();
  }
  template<typename U>
  constexpr bool operator!=(const Reference<U>& b) const {
    return get_alloc() != b.get_alloc();
  }
  template<typename U>
  constexpr bool operator<(const Reference<U>& b) const {
    return get_alloc() < b.get_alloc();
  }

private:
  std::shared_ptr<Node> alloc;
  T* ref;
};

typedef Reference<Node> NodeRef;

struct NodeDrain {
  std::vector<NodeRef> nodes;

  inline size_t size() const {
    return nodes.size();
  }
  template<typename U>
  inline void push(const Reference<U>& node) {
    nodes.emplace_back(node.as<Node>());
  }
};

struct Memory;
struct Type;
struct Expr;
struct Stmt;

typedef Reference<Memory> MemoryRef;
typedef Reference<Type> TypeRef;
typedef Reference<Expr> ExprRef;
typedef Reference<Stmt> StmtRef;
