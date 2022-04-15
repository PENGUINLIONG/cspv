"""
Generate visitor templates for tree-like types.
@PENGUINLIONG
"""

def extract_enum(line, prefix):
    line = line.strip()
    if not line.startswith(prefix):
        return None
    return line[len(prefix):-1]

def extract_enums_from_file(src_path, prefix):
    out = []
    with open(src_path) as f:
        out = [extract_enum(x, prefix) for x in f.readlines()]
        out = [x for x in out if x != None]
    return out

def screaming_snake2pascal(name):
    return ''.join([x.title() for x in name.split('_')])

def compose_general_header(abbr, desc):
    return [
        "// GENERATED SOURCE; DO NOT MODIFY.",
        "// " + desc,
        "// @PENGUINLIONG",
        "#pragma once",
        f'#include "spv/{abbr}.hpp"',
        "",
    ]
def compose_visitor(ty_prefix, enum_prefix, abbr, enum_field, enums):
    """
    For example:
    ty_prefix = Type
    enum_prefix = L_TYPE_CLASS_
    abbr = ty
    enum_field = cls
    enums = ["VOID", "BOOL", ...]
    """
    out = [
        f"struct {ty_prefix}Visitor {{",
        f"  void visit_{abbr}(const {ty_prefix}& {abbr}) {{",
        f"    switch ({abbr}.{enum_field}) {{",
    ]
    for x in enums:
        out += [
            f"    case {enum_prefix}{x}: {abbr}.visit(this); visit_{abbr}(*(const {ty_prefix}{screaming_snake2pascal(x)}*)&{abbr}); break;",
        ]
    out += [
        "    default: liong::unreachable();",
        "    }",
        "  }",
    ]
    for x in enums:
        out += [
            f"  virtual void visit_{abbr}(const {ty_prefix}{screaming_snake2pascal(x)}&) {{}}",
        ]
    out += [
        "};",
        "",
    ]
    return out

TYPES = extract_enums_from_file("./include/spv/type-reg.hpp", "L_TYPE_CLASS_")
out = compose_general_header("type", "Type class visitor.") + \
    compose_visitor("Type", "L_TYPE_CLASS_", "ty", "cls", TYPES)
with open("./include/spv/gen/type-visitor.hpp", "w") as f:
    f.write('\n'.join(out))

EXPRS = extract_enums_from_file("./include/spv/expr-reg.hpp", "L_EXPR_OP_")
out = compose_general_header("expr", "Expression tree visitor.") + \
    compose_visitor("Expr", "L_EXPR_OP_", "expr", "op", EXPRS)
with open("./include/spv/gen/expr-visitor.hpp", "w") as f:
    f.write('\n'.join(out))

STMTS = extract_enums_from_file("./include/spv/stmt-reg.hpp", "L_STMT_OP_")
out = compose_general_header("stmt", "Statement tree visitor.") + \
    compose_visitor("Stmt", "L_STMT_OP_", "stmt", "op", STMTS)
with open("./include/spv/gen/stmt-visitor.hpp", "w") as f:
    f.write('\n'.join(out))
