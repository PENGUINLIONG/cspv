// Simplify integer expressions.
// Depends on `graph-normalization` to ensure the constants are always on the
// right (`b`).
// @PENGUINLIONG
#include "pass/pass.hpp"
#include "visitor/util.hpp"

using namespace liong;

struct IntExprSimplificationMutator : public Mutator {
  virtual ExprRef mutate_expr_(ExprAddRef x) override final {
    // Rotate to make a leftist tree.
    // ```
    //   expr1 const    expr0 expr1
    //      \   /          \   /
    // expr0 add1    =>     add0 const
    //   \   /                \   /
    //    add0                 add1
    // ```
    while (x->b->is<ExprAdd>()) {
      ExprAddRef xb = x->b;
      x = new ExprAdd(
        x->ty,
        new ExprAdd(
          x->ty,
          x->a,
          xb->a
        ),
        xb->b
      );
    }

    // Recursively reduce the tree complexity.
    x = new ExprAdd(
      mutate_ty(x->ty),
      mutate_expr(x->a),
      mutate_expr(x->b)
    );

    /*
    std::vector<ExprRef> terms;
    for (ExprRef x2 = x;;) {
      if (x2->is<ExprAdd>()) {
        ExprAddRef x3 = x2;
        terms.emplace_back(x3->b);
        x2 = x3->a;
      } else if (x2->is<ExprPolynomial>()) {
        ExprPolynomialRef x3 = x2;
        for (const auto& term : x3->terms) {
          terms.emplace_back(term);
        }
      } else {
        terms.emplace_back(x2);
        break;
      }
    }
    ExprPolynomialRef poly = new ExprPolynomial(x->ty, std::move(terms));
    */

    if (x->a->is<ExprAdd>()) {
      ExprAddRef xa = x->a;
      if (xa->b->is<ExprIntImm>()) {
        ExprIntImmRef xab = xa->b;
        if (x->b->is<ExprIntImm>()) {
          ExprIntImmRef xb = x->b;
          // Add up two constants.
          x = new ExprAdd(
            x->ty,
            xa->a,
            new ExprIntImm(x->ty, xab->lit + xb->lit)
          );
        } else {
          // Pull the constant out.
          x = new ExprAdd(
            x->ty,
            new ExprAdd(x->ty, xa->a, x->b),
            new ExprIntImm(x->ty, xab->lit)
          );
        }
      }
    }


    if (x->b->is<ExprIntImm>()) {
      ExprIntImmRef xb = x->b;

      // Add by zero is nop.
      if (xb->lit == 0) {
        return x->a;
      }

      // If the two children are both constants, add them up to a single one.
      if (x->a->is<ExprIntImm>()) {
        ExprIntImmRef xa = x->a;
        return new ExprIntImm(x->ty, xa->lit + xb->lit);
      }
    }

    return x;
  }
  virtual ExprRef mutate_expr_(ExprMulRef x) override final {
    while (x->b->is<ExprMul>()) {
      ExprMulRef xb = x->b;
      x = new ExprMul(
        x->ty,
        new ExprMul(
          x->ty,
          x->a,
          xb->a
        ),
        xb->b
      );
    }

    // Recursively reduce the tree complexity.
    x = new ExprMul(
      mutate_ty(x->ty),
      mutate_expr(x->a),
      mutate_expr(x->b)
    );

    // Push-in constant multiplicants with multiplication's distributive
    // property.
    if (x->a->is<ExprAdd>()) {
      ExprAddRef xa = x->a;
      ExprIntImmRef xb = x->b;
      ExprRef x2 = new ExprAdd(
        x->ty,
        new ExprMul(x->ty, xa->a, xb),
        new ExprMul(x->ty, xa->b, xb)
      );
      return mutate_expr(x2);
    }

    if (x->a->is<ExprMul>()) {
      ExprMulRef xa = x->a;
      if (xa->b->is<ExprIntImm>()) {
        ExprIntImmRef xab = xa->b;
        if (x->b->is<ExprIntImm>()) {
          ExprIntImmRef xb = x->b;
          // Add up two constants.
          x = new ExprMul(
            x->ty,
            xa->a,
            new ExprIntImm(x->ty, xab->lit * xb->lit)
          );
        } else {
          // Pull the constant out.
          x = new ExprMul(
            x->ty,
            new ExprMul(x->ty, xa->a, x->b),
            new ExprIntImm(x->ty, xab->lit)
          );
        }
      }
    }

    if (x->b->is<ExprIntImm>()) {
      ExprIntImmRef xb = x->b;

      // Add by zero is nop.
      if (xb->lit == 1) {
        return x->a;
      }

      // If the two children are both constants, multiply them up to a single
      // one.
      if (x->a->is<ExprIntImm>()) {
        ExprIntImmRef xa = x->a;
        return new ExprIntImm(x->ty, xa->lit * xb->lit);
      }
    }

    return x;
  }
};

struct IntExprSimplificationPass : public Pass {
  IntExprSimplificationPass() : Pass("int-expr-simplification") {}
  virtual void apply(NodeRef& x) override final {
    IntExprSimplificationMutator v;
    x = v.mutate(x);
  }
};
static Pass* PASS = reg_pass<IntExprSimplificationPass>();
