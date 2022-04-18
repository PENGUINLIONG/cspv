// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.
// Memory visitor.
// @PENGUINLIONG
#pragma once
#include "spv/mem.hpp"

struct MemoryVisitor {
  virtual void visit_mem(const Memory& mem) {
    switch (mem.cls) {
    case L_MEMORY_CLASS_FUNCTION_VARIABLE: visit_mem_(*(const MemoryFunctionVariable*)&mem); break;
    case L_MEMORY_CLASS_UNIFORM_BUFFER: visit_mem_(*(const MemoryUniformBuffer*)&mem); break;
    case L_MEMORY_CLASS_STORAGE_BUFFER: visit_mem_(*(const MemoryStorageBuffer*)&mem); break;
    case L_MEMORY_CLASS_SAMPLED_IMAGE: visit_mem_(*(const MemorySampledImage*)&mem); break;
    case L_MEMORY_CLASS_STORAGE_IMAGE: visit_mem_(*(const MemoryStorageImage*)&mem); break;
    default: liong::unreachable();
    }
  }
  virtual void visit_mem_(const MemoryFunctionVariable&);
  virtual void visit_mem_(const MemoryUniformBuffer&);
  virtual void visit_mem_(const MemoryStorageBuffer&);
  virtual void visit_mem_(const MemorySampledImage&);
  virtual void visit_mem_(const MemoryStorageImage&);
};