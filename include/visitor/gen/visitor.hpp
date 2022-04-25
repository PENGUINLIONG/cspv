// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.
// Node visitor and mutator.
// @PENGUINLIONG
#pragma once

#include "node/gen/mem.hpp"
#include "node/gen/ty.hpp"
#include "node/gen/expr.hpp"
#include "node/gen/stmt.hpp"

struct Visitor {
  template<typename T>
  void visit(const Reference<T>& node) {
    switch (node->nova) {
    case L_NODE_VARIANT_MEMORY: visit_mem(node.as<Memory>()); break;
    case L_NODE_VARIANT_TYPE: visit_ty(node.as<Type>()); break;
    case L_NODE_VARIANT_EXPR: visit_expr(node.as<Expr>()); break;
    case L_NODE_VARIANT_STMT: visit_stmt(node.as<Stmt>()); break;
    default: liong::unimplemented();
    }
  }
  inline void visit(const MemoryRef& mem) { return visit_mem(mem); }
  inline void visit(const TypeRef& ty) { return visit_ty(ty); }
  inline void visit(const ExprRef& expr) { return visit_expr(expr); }
  inline void visit(const StmtRef& stmt) { return visit_stmt(stmt); }

  inline void visit_mem(const MemoryRef& mem) {
    switch (mem->cls) {
    case L_MEMORY_CLASS_PATTERN_CAPTURE: visit_mem_(mem.as<MemoryPatternCapture>()); break;
    case L_MEMORY_CLASS_FUNCTION_VARIABLE: visit_mem_(mem.as<MemoryFunctionVariable>()); break;
    case L_MEMORY_CLASS_ITERATION_VARIABLE: visit_mem_(mem.as<MemoryIterationVariable>()); break;
    case L_MEMORY_CLASS_UNIFORM_BUFFER: visit_mem_(mem.as<MemoryUniformBuffer>()); break;
    case L_MEMORY_CLASS_STORAGE_BUFFER: visit_mem_(mem.as<MemoryStorageBuffer>()); break;
    case L_MEMORY_CLASS_SAMPLED_IMAGE: visit_mem_(mem.as<MemorySampledImage>()); break;
    case L_MEMORY_CLASS_STORAGE_IMAGE: visit_mem_(mem.as<MemoryStorageImage>()); break;
    default: liong::unreachable();
    }
  }
  inline void visit_ty(const TypeRef& ty) {
    switch (ty->cls) {
    case L_TYPE_CLASS_PATTERN_CAPTURE: visit_ty_(ty.as<TypePatternCapture>()); break;
    case L_TYPE_CLASS_VOID: visit_ty_(ty.as<TypeVoid>()); break;
    case L_TYPE_CLASS_BOOL: visit_ty_(ty.as<TypeBool>()); break;
    case L_TYPE_CLASS_INT: visit_ty_(ty.as<TypeInt>()); break;
    case L_TYPE_CLASS_FLOAT: visit_ty_(ty.as<TypeFloat>()); break;
    case L_TYPE_CLASS_STRUCT: visit_ty_(ty.as<TypeStruct>()); break;
    case L_TYPE_CLASS_POINTER: visit_ty_(ty.as<TypePointer>()); break;
    default: liong::unreachable();
    }
  }
  inline void visit_expr(const ExprRef& expr) {
    switch (expr->op) {
    case L_EXPR_OP_PATTERN_CAPTURE: visit_expr_(expr.as<ExprPatternCapture>()); break;
    case L_EXPR_OP_PATTERN_BINARY_OP: visit_expr_(expr.as<ExprPatternBinaryOp>()); break;
    case L_EXPR_OP_BOOL_IMM: visit_expr_(expr.as<ExprBoolImm>()); break;
    case L_EXPR_OP_INT_IMM: visit_expr_(expr.as<ExprIntImm>()); break;
    case L_EXPR_OP_FLOAT_IMM: visit_expr_(expr.as<ExprFloatImm>()); break;
    case L_EXPR_OP_LOAD: visit_expr_(expr.as<ExprLoad>()); break;
    case L_EXPR_OP_ADD: visit_expr_(expr.as<ExprAdd>()); break;
    case L_EXPR_OP_SUB: visit_expr_(expr.as<ExprSub>()); break;
    case L_EXPR_OP_MUL: visit_expr_(expr.as<ExprMul>()); break;
    case L_EXPR_OP_DIV: visit_expr_(expr.as<ExprDiv>()); break;
    case L_EXPR_OP_MOD: visit_expr_(expr.as<ExprMod>()); break;
    case L_EXPR_OP_LT: visit_expr_(expr.as<ExprLt>()); break;
    case L_EXPR_OP_EQ: visit_expr_(expr.as<ExprEq>()); break;
    case L_EXPR_OP_NOT: visit_expr_(expr.as<ExprNot>()); break;
    case L_EXPR_OP_TYPE_CAST: visit_expr_(expr.as<ExprTypeCast>()); break;
    case L_EXPR_OP_SELECT: visit_expr_(expr.as<ExprSelect>()); break;
    default: liong::unreachable();
    }
  }
  inline void visit_stmt(const StmtRef& stmt) {
    switch (stmt->op) {
    case L_STMT_OP_PATTERN_CAPTURE: visit_stmt_(stmt.as<StmtPatternCapture>()); break;
    case L_STMT_OP_PATTERN_HEAD: visit_stmt_(stmt.as<StmtPatternHead>()); break;
    case L_STMT_OP_PATTERN_TAIL: visit_stmt_(stmt.as<StmtPatternTail>()); break;
    case L_STMT_OP_NOP: visit_stmt_(stmt.as<StmtNop>()); break;
    case L_STMT_OP_BLOCK: visit_stmt_(stmt.as<StmtBlock>()); break;
    case L_STMT_OP_CONDITIONAL_BRANCH: visit_stmt_(stmt.as<StmtConditionalBranch>()); break;
    case L_STMT_OP_LOOP: visit_stmt_(stmt.as<StmtLoop>()); break;
    case L_STMT_OP_RETURN: visit_stmt_(stmt.as<StmtReturn>()); break;
    case L_STMT_OP_LOOP_MERGE: visit_stmt_(stmt.as<StmtLoopMerge>()); break;
    case L_STMT_OP_LOOP_CONTINUE: visit_stmt_(stmt.as<StmtLoopContinue>()); break;
    case L_STMT_OP_LOOP_BACK_EDGE: visit_stmt_(stmt.as<StmtLoopBackEdge>()); break;
    case L_STMT_OP_RANGED_LOOP: visit_stmt_(stmt.as<StmtRangedLoop>()); break;
    case L_STMT_OP_STORE: visit_stmt_(stmt.as<StmtStore>()); break;
    default: liong::unreachable();
    }
  }

  virtual void visit_mem_(MemoryPatternCaptureRef);
  virtual void visit_mem_(MemoryFunctionVariableRef);
  virtual void visit_mem_(MemoryIterationVariableRef);
  virtual void visit_mem_(MemoryUniformBufferRef);
  virtual void visit_mem_(MemoryStorageBufferRef);
  virtual void visit_mem_(MemorySampledImageRef);
  virtual void visit_mem_(MemoryStorageImageRef);

  virtual void visit_ty_(TypePatternCaptureRef);
  virtual void visit_ty_(TypeVoidRef);
  virtual void visit_ty_(TypeBoolRef);
  virtual void visit_ty_(TypeIntRef);
  virtual void visit_ty_(TypeFloatRef);
  virtual void visit_ty_(TypeStructRef);
  virtual void visit_ty_(TypePointerRef);

  virtual void visit_expr_(ExprPatternCaptureRef);
  virtual void visit_expr_(ExprPatternBinaryOpRef);
  virtual void visit_expr_(ExprBoolImmRef);
  virtual void visit_expr_(ExprIntImmRef);
  virtual void visit_expr_(ExprFloatImmRef);
  virtual void visit_expr_(ExprLoadRef);
  virtual void visit_expr_(ExprAddRef);
  virtual void visit_expr_(ExprSubRef);
  virtual void visit_expr_(ExprMulRef);
  virtual void visit_expr_(ExprDivRef);
  virtual void visit_expr_(ExprModRef);
  virtual void visit_expr_(ExprLtRef);
  virtual void visit_expr_(ExprEqRef);
  virtual void visit_expr_(ExprNotRef);
  virtual void visit_expr_(ExprTypeCastRef);
  virtual void visit_expr_(ExprSelectRef);

  virtual void visit_stmt_(StmtPatternCaptureRef);
  virtual void visit_stmt_(StmtPatternHeadRef);
  virtual void visit_stmt_(StmtPatternTailRef);
  virtual void visit_stmt_(StmtNopRef);
  virtual void visit_stmt_(StmtBlockRef);
  virtual void visit_stmt_(StmtConditionalBranchRef);
  virtual void visit_stmt_(StmtLoopRef);
  virtual void visit_stmt_(StmtReturnRef);
  virtual void visit_stmt_(StmtLoopMergeRef);
  virtual void visit_stmt_(StmtLoopContinueRef);
  virtual void visit_stmt_(StmtLoopBackEdgeRef);
  virtual void visit_stmt_(StmtRangedLoopRef);
  virtual void visit_stmt_(StmtStoreRef);

};

struct Mutator {
  template<typename T>
  NodeRef mutate(const Reference<T>& node) {
    switch (node->nova) {
    case L_NODE_VARIANT_MEMORY: return mutate_mem(node.as<Memory>()).as<Node>();
    case L_NODE_VARIANT_TYPE: return mutate_ty(node.as<Type>()).as<Node>();
    case L_NODE_VARIANT_EXPR: return mutate_expr(node.as<Expr>()).as<Node>();
    case L_NODE_VARIANT_STMT: return mutate_stmt(node.as<Stmt>()).as<Node>();
    default: liong::unimplemented();
    }
  }
  inline MemoryRef mutate(const MemoryRef& mem) { return mutate_mem(mem); }
  inline TypeRef mutate(const TypeRef& ty) { return mutate_ty(ty); }
  inline ExprRef mutate(const ExprRef& expr) { return mutate_expr(expr); }
  inline StmtRef mutate(const StmtRef& stmt) { return mutate_stmt(stmt); }

  inline MemoryRef mutate_mem(const MemoryRef& mem) {
    switch (mem->cls) {
    case L_MEMORY_CLASS_PATTERN_CAPTURE: return mutate_mem_(mem.as<MemoryPatternCapture>());
    case L_MEMORY_CLASS_FUNCTION_VARIABLE: return mutate_mem_(mem.as<MemoryFunctionVariable>());
    case L_MEMORY_CLASS_ITERATION_VARIABLE: return mutate_mem_(mem.as<MemoryIterationVariable>());
    case L_MEMORY_CLASS_UNIFORM_BUFFER: return mutate_mem_(mem.as<MemoryUniformBuffer>());
    case L_MEMORY_CLASS_STORAGE_BUFFER: return mutate_mem_(mem.as<MemoryStorageBuffer>());
    case L_MEMORY_CLASS_SAMPLED_IMAGE: return mutate_mem_(mem.as<MemorySampledImage>());
    case L_MEMORY_CLASS_STORAGE_IMAGE: return mutate_mem_(mem.as<MemoryStorageImage>());
    default: liong::unreachable();
    }
  }
  inline TypeRef mutate_ty(const TypeRef& ty) {
    switch (ty->cls) {
    case L_TYPE_CLASS_PATTERN_CAPTURE: return mutate_ty_(ty.as<TypePatternCapture>());
    case L_TYPE_CLASS_VOID: return mutate_ty_(ty.as<TypeVoid>());
    case L_TYPE_CLASS_BOOL: return mutate_ty_(ty.as<TypeBool>());
    case L_TYPE_CLASS_INT: return mutate_ty_(ty.as<TypeInt>());
    case L_TYPE_CLASS_FLOAT: return mutate_ty_(ty.as<TypeFloat>());
    case L_TYPE_CLASS_STRUCT: return mutate_ty_(ty.as<TypeStruct>());
    case L_TYPE_CLASS_POINTER: return mutate_ty_(ty.as<TypePointer>());
    default: liong::unreachable();
    }
  }
  inline ExprRef mutate_expr(const ExprRef& expr) {
    switch (expr->op) {
    case L_EXPR_OP_PATTERN_CAPTURE: return mutate_expr_(expr.as<ExprPatternCapture>());
    case L_EXPR_OP_PATTERN_BINARY_OP: return mutate_expr_(expr.as<ExprPatternBinaryOp>());
    case L_EXPR_OP_BOOL_IMM: return mutate_expr_(expr.as<ExprBoolImm>());
    case L_EXPR_OP_INT_IMM: return mutate_expr_(expr.as<ExprIntImm>());
    case L_EXPR_OP_FLOAT_IMM: return mutate_expr_(expr.as<ExprFloatImm>());
    case L_EXPR_OP_LOAD: return mutate_expr_(expr.as<ExprLoad>());
    case L_EXPR_OP_ADD: return mutate_expr_(expr.as<ExprAdd>());
    case L_EXPR_OP_SUB: return mutate_expr_(expr.as<ExprSub>());
    case L_EXPR_OP_MUL: return mutate_expr_(expr.as<ExprMul>());
    case L_EXPR_OP_DIV: return mutate_expr_(expr.as<ExprDiv>());
    case L_EXPR_OP_MOD: return mutate_expr_(expr.as<ExprMod>());
    case L_EXPR_OP_LT: return mutate_expr_(expr.as<ExprLt>());
    case L_EXPR_OP_EQ: return mutate_expr_(expr.as<ExprEq>());
    case L_EXPR_OP_NOT: return mutate_expr_(expr.as<ExprNot>());
    case L_EXPR_OP_TYPE_CAST: return mutate_expr_(expr.as<ExprTypeCast>());
    case L_EXPR_OP_SELECT: return mutate_expr_(expr.as<ExprSelect>());
    default: liong::unreachable();
    }
  }
  inline StmtRef mutate_stmt(const StmtRef& stmt) {
    switch (stmt->op) {
    case L_STMT_OP_PATTERN_CAPTURE: return mutate_stmt_(stmt.as<StmtPatternCapture>());
    case L_STMT_OP_PATTERN_HEAD: return mutate_stmt_(stmt.as<StmtPatternHead>());
    case L_STMT_OP_PATTERN_TAIL: return mutate_stmt_(stmt.as<StmtPatternTail>());
    case L_STMT_OP_NOP: return mutate_stmt_(stmt.as<StmtNop>());
    case L_STMT_OP_BLOCK: return mutate_stmt_(stmt.as<StmtBlock>());
    case L_STMT_OP_CONDITIONAL_BRANCH: return mutate_stmt_(stmt.as<StmtConditionalBranch>());
    case L_STMT_OP_LOOP: return mutate_stmt_(stmt.as<StmtLoop>());
    case L_STMT_OP_RETURN: return mutate_stmt_(stmt.as<StmtReturn>());
    case L_STMT_OP_LOOP_MERGE: return mutate_stmt_(stmt.as<StmtLoopMerge>());
    case L_STMT_OP_LOOP_CONTINUE: return mutate_stmt_(stmt.as<StmtLoopContinue>());
    case L_STMT_OP_LOOP_BACK_EDGE: return mutate_stmt_(stmt.as<StmtLoopBackEdge>());
    case L_STMT_OP_RANGED_LOOP: return mutate_stmt_(stmt.as<StmtRangedLoop>());
    case L_STMT_OP_STORE: return mutate_stmt_(stmt.as<StmtStore>());
    default: liong::unreachable();
    }
  }

  virtual MemoryRef mutate_mem_(MemoryPatternCaptureRef);
  virtual MemoryRef mutate_mem_(MemoryFunctionVariableRef);
  virtual MemoryRef mutate_mem_(MemoryIterationVariableRef);
  virtual MemoryRef mutate_mem_(MemoryUniformBufferRef);
  virtual MemoryRef mutate_mem_(MemoryStorageBufferRef);
  virtual MemoryRef mutate_mem_(MemorySampledImageRef);
  virtual MemoryRef mutate_mem_(MemoryStorageImageRef);

  virtual TypeRef mutate_ty_(TypePatternCaptureRef);
  virtual TypeRef mutate_ty_(TypeVoidRef);
  virtual TypeRef mutate_ty_(TypeBoolRef);
  virtual TypeRef mutate_ty_(TypeIntRef);
  virtual TypeRef mutate_ty_(TypeFloatRef);
  virtual TypeRef mutate_ty_(TypeStructRef);
  virtual TypeRef mutate_ty_(TypePointerRef);

  virtual ExprRef mutate_expr_(ExprPatternCaptureRef);
  virtual ExprRef mutate_expr_(ExprPatternBinaryOpRef);
  virtual ExprRef mutate_expr_(ExprBoolImmRef);
  virtual ExprRef mutate_expr_(ExprIntImmRef);
  virtual ExprRef mutate_expr_(ExprFloatImmRef);
  virtual ExprRef mutate_expr_(ExprLoadRef);
  virtual ExprRef mutate_expr_(ExprAddRef);
  virtual ExprRef mutate_expr_(ExprSubRef);
  virtual ExprRef mutate_expr_(ExprMulRef);
  virtual ExprRef mutate_expr_(ExprDivRef);
  virtual ExprRef mutate_expr_(ExprModRef);
  virtual ExprRef mutate_expr_(ExprLtRef);
  virtual ExprRef mutate_expr_(ExprEqRef);
  virtual ExprRef mutate_expr_(ExprNotRef);
  virtual ExprRef mutate_expr_(ExprTypeCastRef);
  virtual ExprRef mutate_expr_(ExprSelectRef);

  virtual StmtRef mutate_stmt_(StmtPatternCaptureRef);
  virtual StmtRef mutate_stmt_(StmtPatternHeadRef);
  virtual StmtRef mutate_stmt_(StmtPatternTailRef);
  virtual StmtRef mutate_stmt_(StmtNopRef);
  virtual StmtRef mutate_stmt_(StmtBlockRef);
  virtual StmtRef mutate_stmt_(StmtConditionalBranchRef);
  virtual StmtRef mutate_stmt_(StmtLoopRef);
  virtual StmtRef mutate_stmt_(StmtReturnRef);
  virtual StmtRef mutate_stmt_(StmtLoopMergeRef);
  virtual StmtRef mutate_stmt_(StmtLoopContinueRef);
  virtual StmtRef mutate_stmt_(StmtLoopBackEdgeRef);
  virtual StmtRef mutate_stmt_(StmtRangedLoopRef);
  virtual StmtRef mutate_stmt_(StmtStoreRef);

};

template<typename T>
struct MemoryFunctorVisitor : public Visitor {
  std::function<void(Reference<T>)> f;
  MemoryFunctorVisitor(std::function<void(Reference<T>)>&& f) :
    f(std::forward<std::function<void(Reference<T>)>>(f)) {}

  virtual void visit_mem_(Reference<T> mem) override final { f(mem); }
};
template<typename T>
void visit_mem_functor(
  std::function<void(Reference<T>)>&& f,
  const Reference<Memory>& x
) {
  MemoryFunctorVisitor<T> visitor(std::forward<std::function<void(Reference<T>)>>(f));
  visitor.visit_mem(x);
}

template<typename T>
struct TypeFunctorVisitor : public Visitor {
  std::function<void(Reference<T>)> f;
  TypeFunctorVisitor(std::function<void(Reference<T>)>&& f) :
    f(std::forward<std::function<void(Reference<T>)>>(f)) {}

  virtual void visit_ty_(Reference<T> ty) override final { f(ty); }
};
template<typename T>
void visit_ty_functor(
  std::function<void(Reference<T>)>&& f,
  const Reference<Type>& x
) {
  TypeFunctorVisitor<T> visitor(std::forward<std::function<void(Reference<T>)>>(f));
  visitor.visit_ty(x);
}

template<typename T>
struct ExprFunctorVisitor : public Visitor {
  std::function<void(Reference<T>)> f;
  ExprFunctorVisitor(std::function<void(Reference<T>)>&& f) :
    f(std::forward<std::function<void(Reference<T>)>>(f)) {}

  virtual void visit_expr_(Reference<T> expr) override final { f(expr); }
};
template<typename T>
void visit_expr_functor(
  std::function<void(Reference<T>)>&& f,
  const Reference<Expr>& x
) {
  ExprFunctorVisitor<T> visitor(std::forward<std::function<void(Reference<T>)>>(f));
  visitor.visit_expr(x);
}

template<typename T>
struct StmtFunctorVisitor : public Visitor {
  std::function<void(Reference<T>)> f;
  StmtFunctorVisitor(std::function<void(Reference<T>)>&& f) :
    f(std::forward<std::function<void(Reference<T>)>>(f)) {}

  virtual void visit_stmt_(Reference<T> stmt) override final { f(stmt); }
};
template<typename T>
void visit_stmt_functor(
  std::function<void(Reference<T>)>&& f,
  const Reference<Stmt>& x
) {
  StmtFunctorVisitor<T> visitor(std::forward<std::function<void(Reference<T>)>>(f));
  visitor.visit_stmt(x);
}

template<typename T>
struct MemoryFunctorMutator : public MemoryMutator {
  typedef Reference<T> TStmtRef;
  std::function<MemoryRef(TStmtRef&)> f;
  MemoryFunctorMutator(std::function<MemoryRef(const TStmtRef&)>&& f) :
    f(std::forward<std::function<MemoryRef(const TStmtRef&)>>(f)) {}

  virtual MemoryRef mutate_mem_(const TStmtRef& mem) override final { return f(mem); }
};
template<typename T>
void mutate_mem_functor(
  std::function<MemoryRef(Reference<T>)>&& f,
  const Memory& x
) {
  MemoryFunctorMutator<T> mutator(std::forward<std::function<MemoryRef(Reference<T>)>>(f));
  return mutator.mutate_mem(x);
}

template<typename T>
struct TypeFunctorMutator : public TypeMutator {
  typedef Reference<T> TStmtRef;
  std::function<TypeRef(TStmtRef&)> f;
  TypeFunctorMutator(std::function<TypeRef(const TStmtRef&)>&& f) :
    f(std::forward<std::function<TypeRef(const TStmtRef&)>>(f)) {}

  virtual TypeRef mutate_ty_(const TStmtRef& ty) override final { return f(ty); }
};
template<typename T>
void mutate_ty_functor(
  std::function<TypeRef(Reference<T>)>&& f,
  const Type& x
) {
  TypeFunctorMutator<T> mutator(std::forward<std::function<TypeRef(Reference<T>)>>(f));
  return mutator.mutate_ty(x);
}

template<typename T>
struct ExprFunctorMutator : public ExprMutator {
  typedef Reference<T> TStmtRef;
  std::function<ExprRef(TStmtRef&)> f;
  ExprFunctorMutator(std::function<ExprRef(const TStmtRef&)>&& f) :
    f(std::forward<std::function<ExprRef(const TStmtRef&)>>(f)) {}

  virtual ExprRef mutate_expr_(const TStmtRef& expr) override final { return f(expr); }
};
template<typename T>
void mutate_expr_functor(
  std::function<ExprRef(Reference<T>)>&& f,
  const Expr& x
) {
  ExprFunctorMutator<T> mutator(std::forward<std::function<ExprRef(Reference<T>)>>(f));
  return mutator.mutate_expr(x);
}

template<typename T>
struct StmtFunctorMutator : public StmtMutator {
  typedef Reference<T> TStmtRef;
  std::function<StmtRef(TStmtRef&)> f;
  StmtFunctorMutator(std::function<StmtRef(const TStmtRef&)>&& f) :
    f(std::forward<std::function<StmtRef(const TStmtRef&)>>(f)) {}

  virtual StmtRef mutate_stmt_(const TStmtRef& stmt) override final { return f(stmt); }
};
template<typename T>
void mutate_stmt_functor(
  std::function<StmtRef(Reference<T>)>&& f,
  const Stmt& x
) {
  StmtFunctorMutator<T> mutator(std::forward<std::function<StmtRef(Reference<T>)>>(f));
  return mutator.mutate_stmt(x);
}
