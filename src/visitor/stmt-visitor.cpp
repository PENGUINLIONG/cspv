#include "visitor/gen/stmt-visitor.hpp"

void StmtVisitor::visit_stmt_(const StmtBlock& x) {
  for (const auto& stmt : x.stmts) {
    visit_stmt(*stmt);
  }
}
void StmtVisitor::visit_stmt_(const StmtConditionalBranch& x) {
  visit_stmt(*x.then_block);
  visit_stmt(*x.else_block);
}
void StmtVisitor::visit_stmt_(const StmtIfThenElse& x) {
  visit_stmt(*x.body_block);
}
void StmtVisitor::visit_stmt_(const StmtLoop& x) {
  visit_stmt(*x.body_block);
  visit_stmt(*x.continue_block);
}
void StmtVisitor::visit_stmt_(const StmtReturn& x) {}
void StmtVisitor::visit_stmt_(const StmtIfThenElseMerge& x) {}
void StmtVisitor::visit_stmt_(const StmtLoopMerge& x) {}
void StmtVisitor::visit_stmt_(const StmtLoopContinue& x) {}
void StmtVisitor::visit_stmt_(const StmtLoopBackEdge& x) {}
void StmtVisitor::visit_stmt_(const StmtStore& x) {}
