#include "visitor/gen/ty-visitor.hpp"

void TypeVisitor::visit_ty_(const TypeVoid& x) {}
void TypeVisitor::visit_ty_(const TypeBool& x) {}
void TypeVisitor::visit_ty_(const TypeInt& x) {}
void TypeVisitor::visit_ty_(const TypeFloat& x) {}
void TypeVisitor::visit_ty_(const TypeStruct& x) {
  for (const auto& member : x.members) {
    visit_ty(*member);
  }
}
void TypeVisitor::visit_ty_(const TypePointer& x) {
  visit_ty(*x.inner);
}
