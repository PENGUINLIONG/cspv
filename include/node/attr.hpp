// Semantically ignorable attributes. Attributes are used by human only.
// @PENGUINLIONG
#pragma once
#include "node/node.hpp"

enum AttributeClass {
  L_ATTRIBUTE_CLASS_FUNCTION_VARIABLE_VALUE,
};

struct Attribute {
  AttributeClass cls;

  Attribute(AttributeClass cls) : cls(cls) {}
};

struct AttributeFunctionVariableValue : public Attribute {
  static const AttributeClass CLS = L_ATTRIBUTE_CLASS_FUNCTION_VARIABLE_VALUE;
  ExprRef value;

  AttributeFunctionVariableValue() : Attribute(CLS) {}
};
