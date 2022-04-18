#include <sstream>
#include "visitor/visitor.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct Debug {
  bool line_start = false;
  std::stringstream s;
  std::string indent;

  std::map<const void*, std::string> var_handle2name_map;

  inline const std::string& get_var_name_by_handle(const void* handle) {
    auto it = var_handle2name_map.find(handle);
    if (it == var_handle2name_map.end()) {
      std::string name = "_";
      name += std::to_string(var_handle2name_map.size());
      var_handle2name_map.emplace(handle, std::move(name));
    }
    return var_handle2name_map.at(handle);
  }

  inline void push_indent() { indent += "  "; }
  inline void pop_indent() { indent.resize(indent.size() - 2); }

  template<typename T>
  inline Debug& operator<<(const T& x) {
    if (line_start) {
      s << indent;
      line_start = false;
    }
    s << x;
    return *this;
  }
  inline Debug& operator<<(std::ostream& (*f)(std::ostream&)) {
    if (f == std::endl<std::stringstream::char_type, std::stringstream::traits_type>) {
      s << indent << "\n";
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
  virtual void visit_mem_(const MemoryFunctionVariableRef& x) override final {
    s << "$" << s.get_var_name_by_handle(x->handle) << ":";
    visit(x->ty);
  }
  virtual void visit_mem_(const MemoryUniformBufferRef& x) override final {
    s << "UniformBuffer@" << x->binding << "," << x->set << "[";
    visit_access_chain(x->ac);
    s << "]:";
    visit(x->ty);
  }



  virtual void visit_ty_(const TypeVoidRef& x) override final {
    s << "void";
  }
  virtual void visit_ty_(const TypeBoolRef& x) override final {
    s << "bool";
  }
  virtual void visit_ty_(const TypeIntRef& x) override final {
    s << (x->is_signed ? "i" : "u") << x->nbit;
  }
  virtual void visit_ty_(const TypeFloatRef& x) override final {
    s << "f" << x->nbit;
  }
  virtual void visit_ty_(const TypeStructRef& x) override final {
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
  virtual void visit_ty_(const TypePointerRef& x) override final {
    s << "Pointer<";
    visit(x->inner);
    s << ">";
  }



  virtual void visit_expr_(const ExprConstantRef& x) override final {
    switch (x->ty->cls) {
    case L_TYPE_CLASS_INT:
    {
      const auto& ty2 = *(const TypeInt*)x->ty.get();
      if (ty2.is_signed) {
        if (ty2.nbit == 32) {
          s << *(const int32_t*)&x->lits[0];
        } else {
          liong::unimplemented();
        }
      } else {
        if (ty2.nbit == 32) {
          s << *(const uint32_t*)&x->lits[0];
        } else {
          liong::unimplemented();
        }
      }
      break;
    }
    case L_TYPE_CLASS_FLOAT:
    {
      const auto& ty2 = *(const TypeFloat*)x->ty.get();
      if (ty2.nbit == 32) {
        s << *(const float*)&x->lits[0];
      } else {
        liong::unimplemented();
      }
      break;
    }
    case L_TYPE_CLASS_BOOL:
    {
      s << (x->lits[0] != 0 ? "true" : "false");
      break;
    }
    default: unimplemented();
    }
    s << ":";
    visit(x->ty);
  }
  virtual void visit_expr_(const ExprLoadRef& x) override final {
    s << "Load(";
    visit(x->src_ptr);
    s << ")";
  }
  virtual void visit_expr_(const ExprAddRef& x) override final {
    s << "(";
    visit(x->a);
    s << " + ";
    visit(x->b);
    s << ")";
  }
  virtual void visit_expr_(const ExprSubRef& x) override final {
    s << "(";
    visit(x->a);
    s << " - ";
    visit(x->b);
    s << ")";
  }
  virtual void visit_expr_(const ExprLtRef& x) override final {
    s << "(";
    visit( x->a);
    s << " < ";
    visit(x->b);
    s << ")";
  }
  virtual void visit_expr_(const ExprEqRef& x) override final {
    s << "(";
    visit(x->a);
    s << " == ";
    visit(x->b);
    s << ")";
  }
  virtual void visit_expr_(const ExprTypeCastRef& x) override final {
    s << "(";
    visit(x->src);
    s << ":";
    visit(x->ty);
    s << ")";
  }



  virtual void visit_stmt_(const StmtBlockRef& x) override final {
    s << "{" << std::endl;
    s.push_indent();
    for (const auto& stmt : x->stmts) {
      visit(stmt);
    }
    s.pop_indent();
    s << "}" << std::endl;
  }
  virtual void visit_stmt_(const StmtConditionalBranchRef& x) override final {
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
  virtual void visit_stmt_(const StmtIfThenElseRef& x) override final {
    visit(x->body_block);
  }
  virtual void visit_stmt_(const StmtLoopRef& x) override final {
    s << "loop {" << std::endl;
    s.push_indent();
    visit(x->body_block);
    s.pop_indent();
    s << "} continue {" << std::endl;
    s.push_indent();
    visit(x->continue_block);
    s.pop_indent();
    s << "}" << std::endl;
  }
  virtual void visit_stmt_(const StmtReturnRef& x) override final {
    s << "return" << std::endl;
  }
  virtual void visit_stmt_(const StmtLoopContinueRef& x) override final {
    s << "continue" << std::endl;
  }
  virtual void visit_stmt_(const StmtLoopBackEdgeRef& x) override final {}
  virtual void visit_stmt_(const StmtIfThenElseMergeRef& x) override final {}
  virtual void visit_stmt_(const StmtLoopMergeRef& x) override final {
    s << "break" << std::endl;
  }
  virtual void visit_stmt_(const StmtRangedLoopRef& x) override final {
    s << "for ";
    visit(x->itervar);
    s << " in range(";
    visit(x->begin);
    s << ", ";
    visit(x->end);
    s << ", ";
    visit(x->stride);
    s << ") {" << std::endl;
    s.push_indent();
    visit(x->body_block);
    s.pop_indent();
    s << "}" << std::endl;
  }
  virtual void visit_stmt_(const StmtStoreRef& x) override final {
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
