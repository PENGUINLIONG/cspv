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
          // Pull the lhs constant out.
          x = new ExprAdd(
            x->ty,
            new ExprAdd(x->ty, xa->a, x->b),
            new ExprIntImm(x->ty, xab->lit)
          );
        }
      }
    } else if (x->a->is<ExprIntImm>()) {
      if (x->b->is<ExprIntImm>()) {
        ExprIntImmRef xa = x->a;
        ExprIntImmRef xb = x->b;
        // Add up two constants.
        return new ExprIntImm(x->ty, xa->lit + xb->lit);
      } else {
        // Flip the node to ensure non-constants are always on the left.
        x = new ExprAdd(x->ty, x->b, x->a);
      }
    } else {
      // Identity.
      if (x->b->is<ExprIntImm>() && x->b->as<ExprIntImm>().lit == 0) {
        return x->a;
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

    if (x->a->is<ExprAdd>()) {
      ExprAddRef xa = x->a;
      if (x->b->is<ExprAdd>()) {
        ExprAddRef xb = x->b;
        // (a + b) * (c + d) = (ac + ad + bc + bd)
        ExprRef x2 = new ExprAdd(
          x->ty,
          new ExprAdd(
            x->ty,
            new ExprMul(x->ty, xa->a, xb->a),
            new ExprMul(x->ty, xa->a, xb->b)
          ),
          new ExprAdd(
            x->ty,
            new ExprMul(x->ty, xa->b, xb->a),
            new ExprMul(x->ty, xa->b, xb->b)
          )
        );
        return mutate_expr(x2);
      } else {
        // Distributive property: c(a + b) = (ac) + (bc)
        ExprRef x2 = new ExprAdd(
          x->ty,
          new ExprMul(x->ty, xa->a, x->b),
          new ExprMul(x->ty, xa->b, x->b)
        );
        return mutate_expr(x2);
      }

    } else if (x->a->is<ExprMul>()) {
      // Note that the rhs is always a non-`ExprMul` node.
      ExprMulRef xa = x->a;
      if (xa->b->is<ExprIntImm>()) {
        ExprIntImmRef xab = xa->b;
        if (x->b->is<ExprIntImm>()) {
          ExprIntImmRef xb = x->b;
          // Multiply two constants.
          x = new ExprMul(
            x->ty,
            xa->a,
            new ExprIntImm(x->ty, xab->lit * xb->lit)
          );
        } else {
          // Pull the lhs constant out.
          x = new ExprMul(
            x->ty,
            new ExprMul(x->ty, xa->a, x->b),
            new ExprIntImm(x->ty, xab->lit)
          );
        }
      }
    } else if (x->a->is<ExprIntImm>()) {
      if (x->b->is<ExprIntImm>()) {
        ExprIntImmRef xa = x->a;
        ExprIntImmRef xb = x->b;
        // Multiply two constants.
        return new ExprIntImm(x->ty, xa->lit * xb->lit);
      } else {
        // Flip the node to ensure non-constants are always on the left.
        x = new ExprMul(x->ty, x->b, x->a);
      }
    } else {
      if (x->b->is<ExprAdd>()) {
        ExprAddRef xb = x->b;
        // Distributive property: c(a + b) = (ac) + (bc)
        ExprRef x2 = new ExprAdd(
          x->ty,
          new ExprMul(x->ty, x->a, xb->a),
          new ExprMul(x->ty, x->a, xb->b)
        );
        return mutate_expr(x2);
      } else if (x->b->is<ExprIntImm>() && x->b->as<ExprIntImm>().lit == 1) {
        // Identity.
        return x->a;
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
