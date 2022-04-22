"""
Generate visitor templates for tree-like types.
@PENGUINLIONG
"""

from typing import Dict, List

class Name:
    def __init__(self, s):
        """Input `s` is in snake case."""
        self.segs = s.split('_')
    def to_spinal_case(self):
        return '-'.join(x.lower() for x in self.segs)
    def to_snake_case(self):
        return '_'.join(x.lower() for x in self.segs)
    def to_screaming_snake_case(self):
        return '_'.join(x.upper() for x in self.segs)
    def to_pascal_case(self):
        return ''.join(x.title() for x in self.segs)

class NodeFieldType:
    def __init__(self, ty):
        is_plural = ty.endswith("[]")
        is_ref_ty = ty[0].isupper()

        if is_plural:
            ty = ty[:-2]
            if is_ref_ty:
                self.field_ty = f"std::vector<NodeRef<{ty}>>"
                self.param_ty = f"const std::vector<NodeRef<{ty}>>&"
            else:
                self.field_ty = f"std::vector<{ty}>"
                self.param_ty = f"std::vector<{ty}>"
        else:
            if is_ref_ty:
                self.field_ty = f"NodeRef<{ty}>"
                self.param_ty = f"const NodeRef<{ty}>&"
            else:
                self.field_ty = f"{ty}"
                self.param_ty = f"{ty}"

        self.raw_name = ty
        self.is_ref_ty = is_ref_ty
        self.is_plural = is_plural

class NodeField:
    def __init__(self, name, ty: NodeFieldType):
        self.name = Name(name)
        self.ty = ty

class NodeSubtype:
    def __init__(self, name, fields: List[NodeField]):
        self.name = Name(name)
        self.fields = fields

class NodeVariant:
    def __init__(self, formal_name, ty_name, ty_abbr, enum_name, enum_abbr, fields: List[NodeField], subtys: List[NodeSubtype]):
        self.formal_name = formal_name
        self.ty_name = Name(ty_name)
        self.ty_abbr = Name(ty_abbr)
        self.enum_name = Name(enum_name)
        self.enum_abbr = Name(enum_abbr)
        self.fields = fields
        self.subtys = subtys



def json2nova(json) -> Dict[str, NodeVariant]:
    out = {}
    for formal_name, nova in json.items():
        ty_name = nova["ty_name"]
        ty_abbr = nova["ty_abbr"]
        enum_name = nova["enum_name"]
        enum_abbr = nova["enum_abbr"]
        variants = nova["variants"]

        common_fields = [NodeField(name, NodeFieldType(ty)) for name, ty in nova["fields"].items()]

        subtys = []
        for name, variant in variants.items():
            fields = [NodeField(name, NodeFieldType(ty)) for name, ty in variant["fields"].items()]
            subtys += [NodeSubtype(name, fields)]

        out[formal_name] = NodeVariant(formal_name, ty_name, ty_abbr, enum_name, enum_abbr, common_fields, subtys)
    return out




def compose_general_header(nova: NodeVariant, desc_appendix):
    desc = f"{nova.formal_name} {desc_appendix}."
    return [
        "// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.",
        f"// {desc}",
        "// @PENGUINLIONG",
        "#pragma once",
    ]
def compose_general_header2(desc: str):
    return [
        "// GENERATED BY `scripts/gen-visitor-templates.py`; DO NOT MODIFY.",
        f"// {desc}.",
        "// @PENGUINLIONG",
        "#pragma once",
    ]

def compose_visitor_hpp(novas: Dict[str, NodeVariant]):
    out = compose_general_header2("Node visitor and mutator")
    out += [""]

    # Include all novas.
    for _, nova in novas.items():
        out += [ f'#include "node/gen/{nova.ty_abbr.to_spinal_case()}.hpp"' ]
    out += [""]

    # Visitor base type.
    out += ["struct Visitor {"]
    # Node traversal basics.
    out += [
        "  template<typename T>",
        "  void visit(const NodeRef<T>& node) {",
        f"    switch (node->nova) {{"
    ]
    for _, nova in novas.items():
        ty_name = nova.ty_name.to_pascal_case()
        abbr = nova.ty_abbr.to_snake_case()
        enum_case_name = "L_NODE_VARIANT_" + nova.ty_name.to_screaming_snake_case()
        out += [f"    case {enum_case_name}: visit_{abbr}(node.as<{ty_name}>()); break;"]
    out += [
        "    default: liong::unimplemented();",
        "    }",
        "  }",
    ]
    for _, nova in novas.items():
        ty_name = nova.ty_name.to_pascal_case()
        abbr = nova.ty_abbr.to_snake_case()
        out += [
            f"  inline void visit(const {ty_name}Ref& {abbr}) {{ return visit_{abbr}({abbr}); }}"
        ]
    out += [""]
    # Typed traversal basics.
    for _, nova in novas.items():
        ty_prefix = nova.ty_name.to_pascal_case()
        enum_prefix = "L_" + nova.ty_name.to_screaming_snake_case() + "_" + nova.enum_name.to_screaming_snake_case() + "_"
        abbr = nova.ty_abbr.to_snake_case()
        enum_var_name = nova.enum_abbr.to_snake_case()
        out += [
            f"  inline void visit_{abbr}(const {ty_prefix}Ref& {abbr}) {{",
            f"    switch ({abbr}->{enum_var_name}) {{",
        ]
        for x in nova.subtys:
            out += [f"    case {enum_prefix}{x.name.to_screaming_snake_case()}: visit_{abbr}_({abbr}.as<{ty_prefix}{x.name.to_pascal_case()}>()); break;"]
        out += [
            "    default: liong::unreachable();",
            "    }",
            "  }",
        ]
    out += [""]
    # Traversal visitor functions.
    for _, nova in novas.items():
        ty_prefix = nova.ty_name.to_pascal_case()
        enum_prefix = "L_" + nova.ty_name.to_screaming_snake_case() + "_" + nova.enum_name.to_screaming_snake_case() + "_"
        abbr = nova.ty_abbr.to_snake_case()
        enum_var_name = nova.enum_abbr.to_snake_case()
        for x in nova.subtys:
            out += [
                f"  virtual void visit_{abbr}_({ty_prefix}{x.name.to_pascal_case()}Ref);",
            ]
        out += [""]
    # End of visitor.
    out += [
        "};",
        "",
    ]

    # Mutator base type.
    out += ["struct Mutator {"]
    # Node traversal basics.
    out += [
        "  template<typename T>",
        "  NodeRef<Node> mutate(const NodeRef<T>& node) {",
        f"    switch (node->nova) {{"
    ]
    for _, nova in novas.items():
        ty_name = nova.ty_name.to_pascal_case()
        abbr = nova.ty_abbr.to_snake_case()
        enum_case_name = "L_NODE_VARIANT_" + nova.ty_name.to_screaming_snake_case()
        out += [f"    case {enum_case_name}: return mutate_{abbr}(node.as<{ty_name}>()).as<Node>();"]
    out += [
        "    default: liong::unimplemented();",
        "    }",
        "  }",
    ]
    for _, nova in novas.items():
        ty_name = nova.ty_name.to_pascal_case()
        abbr = nova.ty_abbr.to_snake_case()
        out += [
            f"  inline {ty_name}Ref mutate(const {ty_name}Ref& {abbr}) {{ return mutate_{abbr}({abbr}); }}"
        ]
    out += [""]
    # Typed traversal basics.
    for _, nova in novas.items():
        ty_prefix = nova.ty_name.to_pascal_case()
        enum_prefix = "L_" + nova.ty_name.to_screaming_snake_case() + "_" + nova.enum_name.to_screaming_snake_case() + "_"
        abbr = nova.ty_abbr.to_snake_case()
        enum_var_name = nova.enum_abbr.to_snake_case()
        out += [
            f"  inline {ty_prefix}Ref mutate_{abbr}(const {ty_prefix}Ref& {abbr}) {{",
            f"    switch ({abbr}->{enum_var_name}) {{",
        ]
        for x in nova.subtys:
            out += [f"    case {enum_prefix}{x.name.to_screaming_snake_case()}: return mutate_{abbr}_({abbr}.as<{ty_prefix}{x.name.to_pascal_case()}>());"]
        out += [
            "    default: liong::unreachable();",
            "    }",
            "  }",
        ]
    out += [""]
    # Traversal visitor functions.
    for _, nova in novas.items():
        ty_prefix = nova.ty_name.to_pascal_case()
        enum_prefix = "L_" + nova.ty_name.to_screaming_snake_case() + "_" + nova.enum_name.to_screaming_snake_case() + "_"
        abbr = nova.ty_abbr.to_snake_case()
        enum_var_name = nova.enum_abbr.to_snake_case()
        for x in nova.subtys:
            subty_prefix = ty_prefix + x.name.to_pascal_case()
            out += [
                f"  virtual {ty_prefix}Ref mutate_{abbr}_({subty_prefix}Ref);",
            ]
        out += [""]
    # End of visitor.
    out += [
        "};",
        "",
    ]

    # Visitor functor.
    for _, nova in novas.items():
        ty_prefix = nova.ty_name.to_pascal_case()
        abbr = nova.ty_abbr.to_snake_case()
        out += [
            f"template<typename T>",
            f"struct {ty_prefix}FunctorVisitor : public Visitor {{",
            f"  std::function<void(NodeRef<T>)> f;",
            f"  {ty_prefix}FunctorVisitor(std::function<void(NodeRef<T>)>&& f) :",
            f"    f(std::forward<std::function<void(NodeRef<T>)>>(f)) {{}}",
            "",
            f"  virtual void visit_{abbr}_(NodeRef<T> {abbr}) override final {{ f({abbr}); }}",
            "};",
            f"template<typename T>",
            f"void visit_{abbr}_functor(",
            f"  std::function<void(NodeRef<T>)>&& f,",
            f"  const NodeRef<{ty_prefix}>& x",
            ") {",
            f"  {ty_prefix}FunctorVisitor<T> visitor(std::forward<std::function<void(NodeRef<T>)>>(f));",
            f"  visitor.visit_{abbr}(x);",
            "}",
            "",
        ]

    # Mutator functor.
    for _, nova in novas.items():
        ty_prefix = nova.ty_name.to_pascal_case()
        abbr = nova.ty_abbr.to_snake_case()
        ty_ref_prefix = ty_prefix + "Ref"
        out += [
            f"template<typename T>",
            f"struct {ty_prefix}FunctorMutator : public {ty_prefix}Mutator {{",
            f"  typedef NodeRef<T> TStmtRef;",
            f"  std::function<{ty_ref_prefix}(TStmtRef&)> f;",
            f"  {ty_prefix}FunctorMutator(std::function<{ty_ref_prefix}(const TStmtRef&)>&& f) :",
            f"    f(std::forward<std::function<{ty_ref_prefix}(const TStmtRef&)>>(f)) {{}}",
            "",
            f"  virtual {ty_ref_prefix} mutate_{abbr}_(const TStmtRef& {abbr}) override final {{ return f({abbr}); }}",
            "};",
            f"template<typename T>",
            f"void mutate_{abbr}_functor(",
            f"  std::function<{ty_ref_prefix}(NodeRef<T>)>&& f,",
            f"  const {ty_prefix}& x",
            ") {",
            f"  {ty_prefix}FunctorMutator<T> mutator(std::forward<std::function<{ty_ref_prefix}(NodeRef<T>)>>(f));",
            f"  return mutator.mutate_{abbr}(x);",
            "}",
            "",
        ]

    with open(f"./include/visitor/gen/visitor.hpp", "w") as f:
        f.write('\n'.join(out))


def compose_enum_reg(nova: NodeVariant):
    enum_name = f"{nova.ty_name.to_pascal_case()}{nova.enum_name.to_pascal_case()}"
    enum_case_prefix = f"L_{nova.ty_name.to_screaming_snake_case()}_{nova.enum_name.to_screaming_snake_case()}_"
    out = [
        f"enum {enum_name} {{",
    ]
    for x in nova.subtys:
        out += [
            f"  {enum_case_prefix}{x.name.to_screaming_snake_case()},"
        ]
    out += [
        "};",
        "",
    ]
    return out

def compose_node_ty_declr(nova: NodeVariant):
    ty_name = nova.ty_name.to_pascal_case()
    enum_name = ty_name + nova.enum_name.to_pascal_case()
    enum_var_name = nova.enum_abbr.to_snake_case()
    out = [
        f"struct {ty_name} : public Node {{",
        f"  const {enum_name} {enum_var_name};",
    ]
    for field in nova.fields:
        out += [
            f"  {field.ty.field_ty} {field.name.to_snake_case()};"
        ]
    out += [
        "",
        "  template<typename T>",
        "  const T& as() const {",
        f'    liong::assert(is<T>(), "{nova.formal_name.lower()} {nova.enum_name.to_spinal_case()} mismatched");',
        "    return *(const T*)this;",
        "  }",
        "  template<typename T>",
        "  bool is() const {",
        f"    return {enum_var_name} == T::{nova.enum_abbr.to_screaming_snake_case()};",
        "  }",
        "",
        "protected:",
        f"  inline {ty_name}(",
        f"    {enum_name} {enum_var_name}",
    ]
    for field in nova.fields:
        out[-1] += ","
        out += [
            f"    {field.ty.param_ty} {field.name.to_snake_case()}",
        ]
    out += [
        f"  ) : Node(L_NODE_VARIANT_{nova.ty_name.to_screaming_snake_case()}), {enum_var_name}({enum_var_name})",
    ]
    for field in nova.fields:
        out[-1] += f", {field.name.to_snake_case()}({field.name.to_snake_case()})"
    out += [
        "  {"
    ]
    for field in nova.fields:
        if field.ty.is_ref_ty:
            if field.ty.is_plural:
                out += [
                    f"    for (const auto& x : {field.name.to_snake_case()}) {{ liong::assert(x != nullptr); }}"
                ]
            else:
                out += [
                    f"    liong::assert({field.name.to_snake_case()} != nullptr);",
                ]
    out += [
        "  }",
        "};",
        "",
    ]
    return out



def compose_reg_hpp(novas: Dict[str, NodeVariant]):
    for _, nova in novas.items():
        out = compose_general_header(nova, "node registry") + \
            [ f'#include "node/node.hpp"', "" ] + \
            compose_enum_reg(nova) + \
            compose_node_ty_declr(nova)
        with open(f"./include/node/gen/{nova.ty_abbr.to_spinal_case()}-reg.hpp", "w") as f:
            f.write('\n'.join(out))



def compose_hpp(novas: Dict[str, NodeVariant]):
    for _, nova in novas.items():

        out = compose_general_header(nova, "node implementation") + \
            [ f'#include "node/reg.hpp"', "" ]

        ty_name = nova.ty_name.to_pascal_case()
        enum_name = ty_name + nova.enum_name.to_pascal_case()
        enum_case_prefix = f"L_{nova.ty_name.to_screaming_snake_case()}_{nova.enum_name.to_screaming_snake_case()}_"

        for subty in nova.subtys:
            subty_name = ty_name + subty.name.to_pascal_case()
            enum_case = enum_case_prefix + subty.name.to_screaming_snake_case()

            # Structure definition.
            out += [
                f"struct {subty_name} : public {ty_name} {{",
                f"  static const {enum_name} {nova.enum_abbr.to_screaming_snake_case()} = {enum_case};",
            ]
            # Fields.
            for field in subty.fields:
                field_name = field.name.to_snake_case()
                out += [f"  {field.ty.field_ty} {field_name};"]
            # Constructor.
            out += [
                "",
                f"  inline {subty_name}(",
            ]
            for i, field in enumerate(nova.fields + subty.fields):
                field_name = field.name.to_snake_case()
                if i != 0:
                    out[-1] += ","
                out += [f"    {field.ty.param_ty} {field_name}"]
            out += [f"  ) : {ty_name}({enum_case}"]
            for field in nova.fields:
                field_name = field.name.to_snake_case()
                out[-1] += f", {field_name}"
            out[-1] += ")"
            for field in subty.fields:
                field_name = field.name.to_snake_case()
                out[-1] += f", {field_name}({field_name})"
            out[-1] += " {"
            for field in subty.fields:
                field_name = field.name.to_snake_case()
                if field.ty.is_ref_ty:
                    if field.ty.is_plural:
                        out += [f"    for (const auto& x : {field_name}) {{ liong::assert(x != nullptr); }}"]
                    else:
                        out += [f"    liong::assert({field_name} != nullptr);"]
            out += [
                "  }",
                "",
                "  virtual void collect_children(NodeDrain* drain) override final {",
            ]
            for field in subty.fields:
                field_name = field.name.to_snake_case()
                if field.ty.is_ref_ty:
                    if field.ty.is_plural:
                        out += [f"    for (const auto& x : {field_name}) {{ drain->push(x); }}"]
                    else:
                        out += [f"    drain->push({field_name});"]
            out += [
                "  }",
                "};",
                ""
            ]

        # Type aliases for convinience.
        ty_prefix = nova.ty_name.to_pascal_case()
        out += [f"typedef NodeRef<{ty_prefix}> {ty_prefix}Ref;"]
        for subty in nova.subtys:
            subty_prefix = subty.name.to_pascal_case()
            out += [f"typedef NodeRef<{ty_prefix}{subty_prefix}> {ty_prefix}{subty_prefix}Ref;"]
        out += [""]


        with open(f"./include/node/gen/{nova.ty_abbr.to_spinal_case()}.hpp", "w") as f:
            f.write('\n'.join(out))



def compose_visitor_cpp(novas: Dict[str, NodeVariant]):

    out = compose_general_header2("Node visitor implementation") + \
        [ f'#include "visitor/gen/visitor.hpp"', "" ]

    # Visitor implementation.
    for _, nova in novas.items():
        ty_name = nova.ty_name.to_pascal_case()
        for subty in nova.subtys:
            subty_name = ty_name + subty.name.to_pascal_case()
            out += [
                f"void Visitor::visit_{nova.ty_abbr.to_snake_case()}_({subty_name}Ref x) {{",
            ]
            for field in subty.fields:
                if field.ty.raw_name == ty_name:
                    if (field.ty.is_plural):
                        out += [
                            f"  for (const auto& x : x->{field.name.to_snake_case()}) {{ visit_{nova.ty_abbr.to_snake_case()}(x); }}",
                        ]
                    else:
                        out += [
                            f"  visit_{nova.ty_abbr.to_snake_case()}(x->{field.name.to_snake_case()});",
                        ]
            out += [
                "}",
            ]
        out += [
            ""
        ]

    # Mutator implementation.
    for _, nova in novas.items():
        ty_name = nova.ty_name.to_pascal_case()
        for subty in nova.subtys:
            subty_name = ty_name + subty.name.to_pascal_case()
            out += [
                f"{ty_name}Ref Mutator::mutate_{nova.ty_abbr.to_snake_case()}_({subty_name}Ref x) {{",
            ]
            for field in subty.fields:
                field_name = field.name.to_snake_case()
                if field.ty.is_ref_ty:
                    if (field.ty.is_plural):
                        out += [
                            f"  for (auto& x : x->{field_name}) {{ x = mutate_{novas[field.ty.raw_name].ty_abbr.to_snake_case()}(x); }}",
                        ]
                    else:
                        out += [
                            f"  x->{field_name} = mutate_{novas[field.ty.raw_name].ty_abbr.to_snake_case()}(x->{field_name});",
                        ]
            out += [
                f"  return x.as<{ty_name}>();",
                "}",
            ]
        out += [
            ""
        ]

        with open(f"./src/visitor/gen/visitor.cpp", "w") as f:
            f.write('\n'.join(out))



novas = {
    "Memory": {
        "ty_name": "memory",
        "ty_abbr": "mem",
        "enum_name": "class",
        "enum_abbr": "cls",
        "fields": {
            "ty": "Type",
            "ac": "Expr[]",
        },
        "variants": {
            "function_variable": {
                "fields": {
                    "handle": "void*",
                }
            },
            "iteration_variable": {
                "fields": {
                    "begin": "Expr",
                    "end": "Expr",
                    "stride": "Expr",
                }
            },
            "uniform_buffer": {
                "fields": {
                    "binding": "uint32_t",
                    "set": "uint32_t",
                }
            },
            "storage_buffer": {
                "fields": {
                    "binding": "uint32_t",
                    "set": "uint32_t",
                }
            },
            "sampled_image": {
                "fields": {
                    "binding": "uint32_t",
                    "set": "uint32_t",
                }
            },
            "storage_image": {
                "fields": {
                    "binding": "uint32_t",
                    "set": "uint32_t",
                }
            },
        },
    },
    "Type": {
        "ty_name": "type",
        "ty_abbr": "ty",
        "enum_name": "class",
        "enum_abbr": "cls",
        "fields": {},
        "variants": {
            "void": {
                "fields": {}
            },
            "bool": {
                "fields": {}
            },
            "int": {
                "fields": {
                    "nbit": "uint32_t",
                    "is_signed": "bool",
                }
            },
            "float": {
                "fields": {
                    "nbit": "uint32_t",
                }
            },
            "struct": {
                "fields": {
                    "members": "Type[]",
                }
            },
            "pointer": {
                "fields": {
                    "inner": "Type",
                }
            },
        },
    },
    "Expr": {
        "ty_name": "expr",
        "ty_abbr": "expr",
        "enum_name": "op",
        "enum_abbr": "op",
        "fields": {
            "ty": "Type",
        },
        "variants": {
            "constant": {
                "fields": {
                    "lits": "uint32_t[]",
                }
            },
            "load": {
                "fields": {
                    "src_ptr": "Memory",
                }
            },
            "add": {
                "fields": {
                    "a": "Expr",
                    "b": "Expr",
                }
            },
            "sub": {
                "fields": {
                    "a": "Expr",
                    "b": "Expr",
                }
            },
            "lt": {
                "fields": {
                    "a": "Expr",
                    "b": "Expr",
                }
            },
            "eq": {
                "fields": {
                    "a": "Expr",
                    "b": "Expr",
                }
            },
            "not": {
                "fields": {
                    "a": "Expr",
                }
            },
            "type_cast": {
                "fields": {
                    "src": "Expr",
                }
            },
        },
    },
    "Stmt": {
        "ty_name": "stmt",
        "ty_abbr": "stmt",
        "enum_name": "op",
        "enum_abbr": "op",
        "fields": {},
        "variants": {
            "nop": {
                "fields": {}
            },
            "block": {
                "fields": {
                    "stmts": "Stmt[]",
                }
            },
            "conditional": {
                "fields": {
                    "cond": "Expr",
                    "then_block": "Stmt",
                }
            },
            "conditional_branch": {
                "fields": {
                    "cond": "Expr",
                    "then_block": "Stmt",
                    "else_block": "Stmt",
                }
            },
            "if_then_else": {
                "fields": {
                    "body_block": "Stmt",
                    "handle": "void*",
                }
            },
            "loop": {
                "fields": {
                    "body_block": "Stmt",
                    "continue_block": "Stmt",
                    "handle": "void*",
                }
            },
            "return": {
                "fields": {}
            },
            "if_then_else_merge": {
                "fields": {
                    "handle": "void*",
                }
            },
            "loop_merge": {
                "fields": {
                    "handle": "void*",
                }
            },
            "loop_continue": {
                "fields": {
                    "handle": "void*",
                }
            },
            "loop_back_edge": {
                "fields": {
                    "handle": "void*",
                }
            },
            "ranged_loop": {
                "fields": {
                    "body_block": "Stmt",
                    "itervar": "Memory",
                }
            },
            "store": {
                "fields": {
                    "dst_ptr": "Memory",
                    "value": "Expr",
                }
            },
        }
    }
}

mem_nova = json2nova(novas)
compose_visitor_hpp(mem_nova)
compose_visitor_cpp(mem_nova)
compose_reg_hpp(mem_nova)
compose_hpp(mem_nova)
