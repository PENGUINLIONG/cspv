#include "visitor/gen/mem-visitor.hpp"

void MemoryVisitor::visit_mem_(const MemoryFunctionVariable&) {}
void MemoryVisitor::visit_mem_(const MemoryUniformBuffer&) {}
void MemoryVisitor::visit_mem_(const MemoryStorageBuffer&) {}
void MemoryVisitor::visit_mem_(const MemorySampledImage&) {}
void MemoryVisitor::visit_mem_(const MemoryStorageImage&) {}
