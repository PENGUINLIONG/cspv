#include "visitor/gen/expr-visitor.hpp"

void ExprVisitor::visit_expr_(const ExprConstant& x) {}
void ExprVisitor::visit_expr_(const ExprLoad& x) {}
void ExprVisitor::visit_expr_(const ExprAdd& x) {
  visit_expr(*x.a);
  visit_expr(*x.b);
}
void ExprVisitor::visit_expr_(const ExprSub& x) {
  visit_expr(*x.a);
  visit_expr(*x.b);
}
void ExprVisitor::visit_expr_(const ExprLt& x) {
  visit_expr(*x.a);
  visit_expr(*x.b);
}
void ExprVisitor::visit_expr_(const ExprEq& x) {
  visit_expr(*x.a);
  visit_expr(*x.b);
}
void ExprVisitor::visit_expr_(const ExprTypeCast& x) {
  visit_expr(*x.src);
}
