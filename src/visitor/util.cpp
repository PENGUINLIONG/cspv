#include <sstream>
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct Debug {
  bool line_start = false;
  std::stringstream s;
  std::string indent;

  std::map<const void*, std::string> var_handle2name_map;

  inline const std::string& get_var_name_by_handle(const std::shared_ptr<uint8_t>& handle) {
    auto it = var_handle2name_map.find(handle.get());
    if (it == var_handle2name_map.end()) {
      std::string name = "_";
      name += std::to_string(var_handle2name_map.size());
      var_handle2name_map.emplace(handle.get(), std::move(name));
    }
    return var_handle2name_map.at(handle.get());
  }

  inline void push_indent() { indent += "  "; }
  inline void pop_indent() { indent.resize(indent.size() - 2); }

  template<typename T>
  inline Debug& operator<<(const T x) {
    if (line_start) {
      s << indent;
      line_start = false;
    }
    s << x;
    return *this;
  }
  inline Debug& operator<<(std::ostream& (*f)(std::ostream&)) {
    if (f == std::endl<std::stringstream::char_type, std::stringstream::traits_type>) {
      s << "\n";
      line_start = true;
      return *this;
    }
    f(s);
    return *this;
  }

  inline std::string str() const { return s.str(); }
};

struct DebugPrintVisitor : public Visitor {
  Debug& s;
  inline DebugPrintVisitor(Debug& s) : s(s) {}

  void visit_access_chain(const std::vector<ExprRef>& ac) {
    bool first = true;
    for (const auto& idx : ac) {
      if (first) {
        first = false;
      } else {
        s << ",";
      }
      visit(idx);
    }
  }
  virtual void visit_mem_(MemoryFunctionVariableRef x) override final {
    s << "$" << s.get_var_name_by_handle(x->handle) << ":";
    visit(x->ty);
  }
  virtual void visit_mem_(MemoryIterationVariableRef x) override final {
    s << "IterVar(";
    visit(x->begin);
    s << ",";
    visit(x->end);
    s << ",";
    visit(x->stride);
    s << "):";
    visit(x->ty);
  }
  virtual void visit_mem_(MemoryUniformBufferRef x) override final {
    s << "UniformBuffer@" << x->binding << "," << x->set << "[";
    visit_access_chain(x->ac);
    s << "]:";
    visit(x->ty);
  }



  virtual void visit_ty_(TypeVoidRef x) override final {
    s << "void";
  }
  virtual void visit_ty_(TypeBoolRef x) override final {
    s << "bool";
  }
  virtual void visit_ty_(TypeIntRef x) override final {
    s << (x->is_signed ? "i" : "u") << x->nbit;
  }
  virtual void visit_ty_(TypeFloatRef x) override final {
    s << "f" << x->nbit;
  }
  virtual void visit_ty_(TypeStructRef x) override final {
    s << "Struct<";
    bool first = true;
    for (const auto& member : x->members) {
      if (first) {
        first = false;
      } else {
        s << ",";
      }
      visit(member);
    }
    s << ">";
  }
  virtual void visit_ty_(TypePointerRef x) override final {
    s << "Pointer<";
    visit(x->inner);
    s << ">";
  }



  virtual void visit_expr_(ExprBoolImmRef x) override final {
    s << (x->lit ? "true" : "false");
  }
  virtual void visit_expr_(ExprIntImmRef x) override final {
    s << x->lit;
  }
  virtual void visit_expr_(ExprFloatImmRef x) override final {
    s << x->lit;
  }
  virtual void visit_expr_(ExprLoadRef x) override final {
    s << "Load(";
    visit(x->src_ptr);
    s << ")";
  }
  virtual void visit_expr_(ExprAddRef x) override final {
    s << "(";
    visit(x->a);
    s << " + ";
    visit(x->b);
    s << ")";
  }
  virtual void visit_expr_(ExprSubRef x) override final {
    s << "(";
    visit(x->a);
    s << " - ";
    visit(x->b);
    s << ")";
  }
  virtual void visit_expr_(ExprLtRef x) override final {
    s << "(";
    visit( x->a);
    s << " < ";
    visit(x->b);
    s << ")";
  }
  virtual void visit_expr_(ExprEqRef x) override final {
    s << "(";
    visit(x->a);
    s << " == ";
    visit(x->b);
    s << ")";
  }
  virtual void visit_expr_(ExprNotRef x) override final {
    s << "!";
    visit(x->a);
    s << "";
  }
  virtual void visit_expr_(ExprTypeCastRef x) override final {
    s << "(";
    visit(x->src);
    s << ":";
    visit(x->ty);
    s << ")";
  }
  virtual void visit_expr_(ExprSelectRef x) override final {
    s << "(";
    visit(x->cond);
    s << "?";
    visit(x->a);
    s << ":";
    visit(x->b);
    s << ")";
  }


  virtual void visit_stmt_(StmtNopRef x) override final {
    s << "nop" << std::endl;
  }
  virtual void visit_stmt_(StmtBlockRef x) override final {
    s << "{" << std::endl;
    s.push_indent();
    for (const auto& stmt : x->stmts) {
      visit(stmt);
    }
    s.pop_indent();
    s << "}" << std::endl;
  }
  virtual void visit_stmt_(StmtConditionalBranchRef x) override final {
    s << "if ";
    visit(x->cond);
    s << " {" << std::endl;
    s.push_indent();
    visit(x->then_block);
    s.pop_indent();
    s << "} else {" << std::endl;
    s.push_indent();
    visit(x->else_block);
    s.pop_indent();
    s << "}" << std::endl;
  }
  virtual void visit_stmt_(StmtLoopRef x) override final {
    s << "loop@" << s.get_var_name_by_handle(x->handle) << " {" << std::endl;
    s.push_indent();
    visit(x->body_block);
    s.pop_indent();
    s << "} continue@" << s.get_var_name_by_handle(x->handle) << " {" << std::endl;
    s.push_indent();
    visit(x->continue_block);
    s.pop_indent();
    s << "}" << std::endl;
  }
  virtual void visit_stmt_(StmtReturnRef x) override final {
    s << "return" << std::endl;
  }
  virtual void visit_stmt_(StmtLoopContinueRef x) override final {
    s << "continue@" << s.get_var_name_by_handle(x->handle) << std::endl;
  }
  virtual void visit_stmt_(StmtLoopBackEdgeRef x) override final {
    s << "back-edge@" << s.get_var_name_by_handle(x->handle) << std::endl;
  }
  virtual void visit_stmt_(StmtLoopMergeRef x) override final {
    s << "break@" << s.get_var_name_by_handle(x->handle) << std::endl;
  }
  virtual void visit_stmt_(StmtRangedLoopRef x) override final {
    s << "for ";
    visit(x->itervar);
    s << " {" << std::endl;
    s.push_indent();
    visit(x->body_block);
    s.pop_indent();
    s << "}" << std::endl;
  }
  virtual void visit_stmt_(StmtStoreRef x) override final {
    s << "Store(";
    visit(x->dst_ptr);
    s << ", ";
    visit(x->value);
    s << ")";
    s << std::endl;
  }
};

std::string dbg_print(const NodeRef& node) {
  Debug s;
  DebugPrintVisitor v(s);
  v.visit(node);
  return s.str();
}

bool is_tail_stmt(const StmtRef& x) {
  switch (x->op) {
  case L_STMT_OP_BLOCK:
    return is_tail_stmt(x->as<StmtBlock>().stmts.back());
  case L_STMT_OP_CONDITIONAL_BRANCH:
  {
    const auto& stmt = x->as<StmtConditionalBranch>();
    return is_tail_stmt(stmt.then_block) && is_tail_stmt(stmt.else_block);
  }
  case L_STMT_OP_RETURN:
  case L_STMT_OP_LOOP_MERGE:
  case L_STMT_OP_LOOP_CONTINUE:
  case L_STMT_OP_LOOP_BACK_EDGE:
    return true;
  default:
    return false;
  }
}
StmtRef flatten_block(const StmtRef& x) {
  if (!x->is<StmtBlock>()) { return x; }
  auto block = x.as<StmtBlock>();

  std::vector<StmtRef> stmts;
  stmts.reserve(block->stmts.size());
  for (const StmtRef& stmt : block->stmts) {
    if (stmt->is<StmtBlock>()) {
      const auto& stmts2 = stmt.as<StmtBlock>()->stmts;
      // If it's a nested block, flatten its content to remove indirection.
      for (auto stmt : stmts2) {
        stmts.emplace_back(stmt);
      }
    } else if (stmt->is<StmtNop>()) {
      // Ignore nops.
      continue;
    } else {
      stmts.emplace_back(stmt);
    }

    // If it's a tail statement, any other statement after it becomes dead
    // code and will never be reached. So it's okay to ignore them.
    if (is_tail_stmt(stmts.back())) { break; }
  }

  switch (stmts.size()) {
  case 0: return new StmtNop();
  case 1: return std::move(stmts[0]);
  default: return new StmtBlock(std::move(stmts));
  }
}

StmtRef& get_head_stmt(StmtRef& stmt) {
  if (stmt->is<StmtBlock>()) {
    auto& block = stmt->as<StmtBlock>();
    assert(!block.stmts.empty());
    return get_tail_stmt((StmtRef&)block.stmts.front());
  } else {
    return stmt;
  }
}

StmtRef& get_tail_stmt(StmtRef& stmt) {
  if (stmt->is<StmtBlock>()) {
    auto& block = stmt->as<StmtBlock>();
    assert(!block.stmts.empty());
    return get_tail_stmt((StmtRef&)block.stmts.back());
  } else {
    return stmt;
  }
}

std::vector<NodeRef> collect_children(const NodeRef& node) {
  NodeDrain drain;
  node->collect_children(&drain);
  return drain.nodes;
}

bool match_pattern(const NodeRef& pattern, const NodeRef& target) {
  if (pattern->nova != target->nova) {
    return false;
  }
  switch (pattern->nova) {
  case L_NODE_VARIANT_TYPE:
  {
    auto pattern2 = pattern.as<Type>();
    if (pattern2->cls == L_TYPE_CLASS_PATTERN_CAPTURE) {
      auto pattern3 = pattern2.as<TypePatternCapture>();
      if (pattern3->captured == nullptr) {
        pattern3->captured = target.as<Type>();
        return true;
      } else {
        return match_pattern(pattern3->captured, target);
      }
    } else if (pattern2->cls != target.as<Type>()->cls) {
      return false;
    } else {
      break;
    }
  }
  case L_NODE_VARIANT_MEMORY:
  {
    auto pattern2 = pattern.as<Memory>();
    if (pattern2->cls == L_MEMORY_CLASS_PATTERN_CAPTURE) {
      auto pattern3 = pattern2.as<MemoryPatternCapture>();
      if (pattern3->captured == nullptr) {
        pattern3->captured = target.as<Memory>();
        return true;
      } else {
        return match_pattern(pattern3->captured, target);
      }
    } else if (pattern2->cls != target.as<Memory>()->cls) {
      return false;
    } else {
      break;
    }
  }
  case L_NODE_VARIANT_EXPR:
  {
    auto pattern2 = pattern.as<Expr>();
    if (pattern2->op == L_EXPR_OP_PATTERN_CAPTURE) {
      auto pattern3 = pattern2.as<ExprPatternCapture>();
      if (pattern3->captured == nullptr) {
        pattern3->captured = target.as<Expr>();
        return true;
      } else {
        return match_pattern(pattern3->captured, target);
      }
    } else if (pattern2->op == L_EXPR_OP_PATTERN_BINARY_OP) {
      auto pattern3 = pattern2.as<ExprPatternBinaryOp>();
      auto target2 = target.as<Expr>();
      if (!is_expr_binary_op(target2->op)) { return false; }

      auto operands = collect_children(target2);
      bool out = true;
      if (pattern3->op == nullptr) {
        pattern3->op = std::make_shared<ExprOp>(target2->op);
      } else {
        out &= *pattern3->op == target2->op;
      }
      if (pattern3->ty == nullptr) {
        pattern3->ty = operands[0];
      } else {
        out &= match_pattern(pattern3->ty, operands[0]);
      }
      if (pattern3->a == nullptr) {
        pattern3->a = operands[1];
      } else {
        out &= match_pattern(pattern3->a, operands[1]);
      }
      if (pattern3->b == nullptr) {
        pattern3->b = operands[1];
      } else {
        out &= match_pattern(pattern3->b, operands[2]);
      }
      return out;
    } else if (pattern2->op == L_EXPR_OP_INT_IMM) {
      auto pattern3 = pattern2.as<ExprIntImm>();
      auto target2 = target.as<ExprIntImm>();

      if (target2->op != L_EXPR_OP_INT_IMM) {
        return false;
      }

      bool out = true;
      if (pattern3->ty == nullptr) {
        pattern3->ty = target2->ty;
        pattern3->lit = target2->lit;
      } else {
        out &= match_pattern(pattern3->ty, target2->ty);
        out &= pattern3->lit == target2->lit;
      }
      return out;
    } else if (pattern2->op == L_EXPR_OP_FLOAT_IMM) {
      auto pattern3 = pattern2.as<ExprFloatImm>();
      auto target2 = target.as<ExprFloatImm>();

      if (target2->op != L_EXPR_OP_FLOAT_IMM) {
        return false;
      }

      bool out = true;
      if (pattern3->ty == nullptr) {
        pattern3->ty = target2->ty;
        pattern3->lit = target2->lit;
      } else {
        out &= match_pattern(pattern3->ty, target2->ty);
        out &= pattern3->lit == target2->lit;
      }
      return out;
    } else if (pattern2->op != target.as<Expr>()->op) {
      return false;
    } else {
      break;
    }
  }
  case L_NODE_VARIANT_STMT:
  {
    auto pattern2 = pattern.as<Stmt>();
    auto target2 = target.as<Stmt>();
    if (pattern2->op == L_STMT_OP_PATTERN_CAPTURE) {
      auto pattern3 = pattern2.as<StmtPatternCapture>();
      if (pattern3->captured == nullptr) {
        pattern3->captured = target.as<Stmt>();
        return true;
      } else {
        return match_pattern(pattern3->captured, target);
      }
    } else if (pattern2->op == L_STMT_OP_PATTERN_HEAD) {
      auto pattern3 = pattern2.as<StmtPatternHead>();
      if (target2->is<StmtBlock>()) {
        return match_pattern(pattern3->inner, target2->as<StmtBlock>().stmts.front().as<Node>());
      } else {
        return match_pattern(pattern3->inner, target2);
      }
    } else if (pattern2->op == L_STMT_OP_PATTERN_TAIL) {
      auto pattern3 = pattern2.as<StmtPatternTail>();
      if (target2->is<StmtBlock>()) {
        return match_pattern(pattern3->inner, target2->as<StmtBlock>().stmts.back().as<Node>());
      } else {
        return match_pattern(pattern3->inner, target2);
      }
    } else if (pattern2->op != target.as<Stmt>()->op) {
      return false;
    } else {
      break;
    }
  }
  default: liong::unreachable();
  }

  auto pattern_nodes = collect_children(pattern);
  auto target_nodes = collect_children(target);
  if (pattern_nodes.size() != target_nodes.size()) {
    return false;
  }
  for (size_t i = 0; i < pattern_nodes.size(); ++i) {
    if (!match_pattern(pattern_nodes[i], target_nodes[i])) {
      return false;
    }
  }

  return true;
}

PredefinedType get_predefined_ty(const TypeRef& ty) {
  switch (ty->cls) {
  case L_TYPE_CLASS_BOOL:
  {
    return L_PREDEFINED_TYPE_BOOL;
  }
  case L_TYPE_CLASS_INT:
  {
    const auto& ty2 = ty->as<TypeInt>();
    if (ty2.nbit == 32) {
      if (ty2.is_signed) {
        return L_PREDEFINED_TYPE_I32;
      } else {
        return L_PREDEFINED_TYPE_U32;
      }
    } else if (ty2.nbit == 64) {
      if (ty2.is_signed) {
        return L_PREDEFINED_TYPE_I64;
      } else {
        return L_PREDEFINED_TYPE_U64;
      }
    }
    break;
  }
  case L_TYPE_CLASS_FLOAT:
  {
    const auto& ty2 = ty->as<TypeFloat>();
    if (ty2.nbit == 32) {
      return L_PREDEFINED_TYPE_F32;
    } else if (ty2.nbit == 64) {
      return L_PREDEFINED_TYPE_F64;
    }
    break;
  }
  }
  return L_PREDEFINED_TYPE_UNDEFINED;
}
