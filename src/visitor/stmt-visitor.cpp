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
void StmtVisitor::visit_stmt_(const StmtRangedLoop& x) {
  visit_stmt(*x.body_block);
}
void StmtVisitor::visit_stmt_(const StmtStore& x) {}



std::shared_ptr<Stmt> StmtMutator::mutate_stmt_(std::shared_ptr<StmtBlock>& x) {
  for (auto& stmt : x->stmts) {
    stmt = mutate_stmt(stmt);
  }
  return x;
}
std::shared_ptr<Stmt> StmtMutator::mutate_stmt_(std::shared_ptr<StmtConditionalBranch>& x) {
  x->then_block = mutate_stmt(x->then_block);
  x->else_block = mutate_stmt(x->else_block);
  return x;
}
std::shared_ptr<Stmt> StmtMutator::mutate_stmt_(std::shared_ptr<StmtIfThenElse>& x) {
  x->body_block = mutate_stmt(x->body_block);
  return x;
}
std::shared_ptr<Stmt> StmtMutator::mutate_stmt_(std::shared_ptr<StmtLoop>& x) {
  x->body_block = mutate_stmt(x->body_block);
  x->continue_block = mutate_stmt(x->continue_block);
  return x;
}
std::shared_ptr<Stmt> StmtMutator::mutate_stmt_(std::shared_ptr<StmtReturn>& x) {
  return x;
}
std::shared_ptr<Stmt> StmtMutator::mutate_stmt_(std::shared_ptr<StmtIfThenElseMerge>& x) {
  return x;
}
std::shared_ptr<Stmt> StmtMutator::mutate_stmt_(std::shared_ptr<StmtLoopMerge>& x) {
  return x;
}
std::shared_ptr<Stmt> StmtMutator::mutate_stmt_(std::shared_ptr<StmtLoopContinue>& x) {
  return x;
}
std::shared_ptr<Stmt> StmtMutator::mutate_stmt_(std::shared_ptr<StmtLoopBackEdge>& x) {
  return x;
}
std::shared_ptr<Stmt> StmtMutator::mutate_stmt_(std::shared_ptr<StmtRangedLoop>& x) {
  x->body_block = mutate_stmt(x->body_block);
  return x;
}
std::shared_ptr<Stmt> StmtMutator::mutate_stmt_(std::shared_ptr<StmtStore>& x) {
  return x;
}
