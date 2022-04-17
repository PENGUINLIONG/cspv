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

  void visit_access_chain(const AccessChain& ac) {
    bool first = true;
    for (const auto& idx : ac.idxs) {
      if (first) {
        first = false;
      } else {
        s << ",";
      }
      visit(*idx);
    }
  }
  virtual void visit_mem_(const MemoryFunctionVariable& x) override final {
    s << "$" << s.get_var_name_by_handle(x.handle) << ":";
    visit(*x.ty);
  }
  virtual void visit_mem_(const MemoryUniformBuffer& x) override final {
    s << "UniformBuffer@" << x.binding << "," << x.set << "[";
    visit_access_chain(x.ac);
    s << "]:";
    visit(*x.ty);
  }



  virtual void visit_ty_(const TypeVoid& x) override final {
    s << "void";
  }
  virtual void visit_ty_(const TypeBool& x) override final {
    s << "bool";
  }
  virtual void visit_ty_(const TypeInt& x) override final {
    s << (x.is_signed ? "i" : "u") << x.nbit;
  }
  virtual void visit_ty_(const TypeFloat& x) override final {
    s << "f" << x.nbit;
  }
  virtual void visit_ty_(const TypeStruct& x) override final {
    s << "Struct<";
    bool first = true;
    for (const auto& member : x.members) {
      if (first) {
        first = false;
      } else {
        s << ",";
      }
      visit(*member);
    }
    s << ">";
  }
  virtual void visit_ty_(const TypePointer& x) override final {
    s << "Pointer<";
    visit(*x.inner);
    s << ">";
  }



  virtual void visit_expr_(const ExprConstant& x) override final {
    switch (x.ty->cls) {
    case L_TYPE_CLASS_INT:
    {
      const auto& ty2 = *(const TypeInt*)x.ty.get();
      if (ty2.is_signed) {
        if (ty2.nbit == 32) {
          s << *(const int32_t*)&x.lits[0];
        } else {
          liong::unimplemented();
        }
      } else {
        if (ty2.nbit == 32) {
          s << *(const uint32_t*)&x.lits[0];
        } else {
          liong::unimplemented();
        }
      }
      break;
    }
    case L_TYPE_CLASS_FLOAT:
    {
      const auto& ty2 = *(const TypeFloat*)x.ty.get();
      if (ty2.nbit == 32) {
        s << *(const float*)&x.lits[0];
      } else {
        liong::unimplemented();
      }
      break;
    }
    case L_TYPE_CLASS_BOOL:
    {
      s << (x.lits[0] != 0 ? "true" : "false");
      break;
    }
    default: unimplemented();
    }
    s << ":";
    visit(*x.ty);
  }
  virtual void visit_expr_(const ExprLoad& x) override final {
    s << "Load(";
    visit(*x.src_ptr);
    s << ")";
  }
  virtual void visit_expr_(const ExprAdd& x) override final {
    s << "(";
    visit(*x.a);
    s << " + ";
    visit(*x.b);
    s << ")";
  }
  virtual void visit_expr_(const ExprSub& x) override final {
    s << "(";
    visit(*x.a);
    s << " - ";
    visit(*x.b);
    s << ")";
  }
  virtual void visit_expr_(const ExprLt& x) override final {
    s << "(";
    visit( *x.a);
    s << " < ";
    visit(*x.b);
    s << ")";
  }
  virtual void visit_expr_(const ExprEq& x) override final {
    s << "(";
    visit(*x.a);
    s << " == ";
    visit(*x.b);
    s << ")";
  }
  virtual void visit_expr_(const ExprTypeCast& x) override final {
    s << "(";
    visit(*x.src);
    s << ":";
    visit(*x.ty);
    s << ")";
  }



  virtual void visit_stmt_(const StmtBlock& x) override final {
    s << "{" << std::endl;
    s.push_indent();
    for (const auto& stmt : x.stmts) {
      visit(*stmt);
    }
    s.pop_indent();
    s << "}" << std::endl;
  }
  virtual void visit_stmt_(const StmtConditionalBranch& x) override final {
    s << "if ";
    visit(*x.cond);
    s << " " << std::endl;
    visit(*x.then_block);
    s << "else " << std::endl;
    visit(*x.else_block);
  }
  virtual void visit_stmt_(const StmtIfThenElse& x) override final {
    visit(*x.body_block);
  }
  virtual void visit_stmt_(const StmtLoop& x) override final {
    s << "loop " << std::endl;
    visit(*x.body_block);
    s << "continue " << std::endl;
    visit(*x.continue_block);
  }
  virtual void visit_stmt_(const StmtReturn& x) override final {
    s << "return ";
    if (x.rv) {
      visit(*x.rv);
    }
    s << std::endl;
  }
  virtual void visit_stmt_(const StmtLoopContinue& x) override final {
    s << "continue" << std::endl;
  }
  virtual void visit_stmt_(const StmtLoopBackEdge& x) override final {}
  virtual void visit_stmt_(const StmtIfThenElseMerge& x) override final {}
  virtual void visit_stmt_(const StmtLoopMerge& x) override final {
    s << "break" << std::endl;
  }
  virtual void visit_stmt_(const StmtRangedLoop& x) override final {
    s << "for ";
    visit(*x.itervar);
    s << " in range(";
    visit(*x.begin);
    s << ", ";
    visit(*x.end);
    s << ", ";
    visit(*x.stride);
    s << ") ";
    visit(*x.body_block);
  }
  virtual void visit_stmt_(const StmtStore& x) override final {
    s << "Store(";
    visit(*x.dst_ptr);
    s << ", ";
    visit(*x.value);
    s << ")";
    s << std::endl;
  }
};

std::string dbg_print(const Node& node) {
  Debug s;
  DebugPrintVisitor v(s);
  v.visit(node);
  return s.str();
}
