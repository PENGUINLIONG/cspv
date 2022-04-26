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
      ExprIntImmRef xa = x->a;
      if (xa->lit == 0) {
        return new ExprIntImm(x->ty, 0);
      } else if (x->b->is<ExprIntImm>()) {
        ExprIntImmRef xb = x->b;
        // Multiply two constants.
        return new ExprIntImm(x->ty, xa->lit * xb->lit);
      } else {
        // Flip the node to ensure non-constants are always on the left.
        x = new ExprMul(x->ty, x->b, xa);
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
      } else if (x->b->is<ExprIntImm>()) {
        ExprIntImmRef xb = x->b;
        if (xb->as<ExprIntImm>().lit == 0) {
          // Zero.
          return new ExprIntImm(x->ty, 0);
        } else if (xb->as<ExprIntImm>().lit == 1) {
          // Identity.
          return x->a;
        }
      }
    }

    return x;
  }

  struct SolvePolynomialCoeGcd {
    int64_t gcd;

    int64_t get_term_coe(ExprRef a) {
      if (a->is<ExprIntImm>()) {
        ExprIntImmRef a2 = a;
        return a2->lit;

      } else if (a->is<ExprMul>()) {
        ExprMulRef a2 = a;
        if (a2->b->is<ExprIntImm>()) {
          ExprIntImmRef ab = a2->b;
          return ab->lit;
        }
      } else {
      }
      return 0;
    }
    int64_t solve_gcd(int64_t a, int64_t b) {
      int64_t c;
      while (b != 0) {
        c = std::remainder(a, b);
        a = b;
        b = c;
      }
      return a;
    }
    void visit_add(ExprAddRef x) {
      if (x->a->is<ExprAdd>()) {
        visit_add(x->a);
        if (gcd != 0) {
          gcd = solve_gcd(gcd, get_term_coe(x->b));
        }
      } else {
        gcd = solve_gcd(get_term_coe(x->a), get_term_coe(x->b));
      }
    }
    int64_t solve(ExprRef x, int64_t divisor) {
      if (!x->is<ExprAdd>() && !x->is<ExprMul>()) { return 0; }
      visit_add(x);
      return solve_gcd(gcd, divisor);
    }
  };
  virtual ExprRef mutate_expr_(ExprDivRef x) override final {
    if (!x->b->is<ExprIntImm>()) { return x; }
    ExprIntImmRef xb = x->b;

    // Keep division-by-zero as-is.
    int64_t divisor = xb->lit;
    if (divisor == 0) { return x; }

    if (x->a->is<ExprIntImm>()) {
      ExprIntImmRef xa = x->a;
      return new ExprIntImm(x->ty, xa->lit / divisor);
    } else if (x->a->is<ExprMul>()) {
      // Divisor must equal GCD otherwise pushing the division in would generate
      // more division computation.
      if (SolvePolynomialCoeGcd().solve(x->a, divisor) == divisor) {
        ExprMulRef xa = x->a;

        if (xa->b->is<ExprIntImm>()) {
          ExprIntImmRef xab = xa->b;

          ExprRef x2 = new ExprMul(
            x->ty,
            xa->a,
            new ExprIntImm(x->ty, xab->lit / divisor)
          );
          return mutate_expr(x2);
        }
      }
    } else if (x->a->is<ExprAdd>()) {
      if (SolvePolynomialCoeGcd().solve(x->a, divisor) == divisor) {
        ExprAddRef xa = x->a;

        if (xa->b->is<ExprIntImm>()) {
          ExprIntImmRef xab = xa->b;

          ExprRef x2 = new ExprAdd(
            x->ty,
            new ExprDiv(
              x->ty,
              xa->a,
              xb
            ),
            new ExprIntImm(x->ty, xab->lit / divisor)
          );
          return mutate_expr(x2);

        } else if (xa->b->is<ExprMul>()) {
          ExprMulRef xab = xa->b;
          if (xab->b->is<ExprIntImm>()) {
            ExprIntImmRef xabb = xab->b;

            ExprRef x2 = new ExprAdd(
              x->ty,
              new ExprDiv(
                x->ty,
                xa->a,
                xb
              ),
              new ExprMul(
                x->ty,
                xab->a,
                new ExprIntImm(x->ty, xabb->lit / divisor)
              )
            );
            return mutate_expr(x2);
          }
        }

      }
    }

    x = new ExprDiv(
      mutate_ty(x->ty),
      mutate_expr(x->a),
      mutate_expr(x->b)
    );
    return x;
  }
  virtual ExprRef mutate_expr_(ExprModRef x) override final {
    if (!x->b->is<ExprIntImm>()) { return x; }
    ExprIntImmRef xb = x->b;

    // Keep division-by-zero as-is.
    int64_t divisor = xb->lit;
    if (divisor == 0) { return x; }

    if (x->a->is<ExprIntImm>()) {
      ExprIntImmRef xa = x->a;
      return new ExprIntImm(x->ty, xa->lit % divisor);
    } else if (x->a->is<ExprMul>()) {
      ExprMulRef xa = x->a;

      if (xa->b->is<ExprIntImm>()) {
        ExprIntImmRef xab = xa->b;

        if (xab->lit % divisor == 0) {
          return new ExprIntImm(x->ty, 0);
        }
      }
    } else if (x->a->is<ExprMod>()) {
      ExprModRef xa = x->a;

      if (xa->b->is<ExprIntImm>()) {
        ExprIntImmRef xab = xa->b;

        if (xab->lit % divisor == 0) {
          x = new ExprMod(x->ty, xa->a, xb);
          return mutate_expr(x);
        } else if (divisor % xab->lit == 0) {
          x = new ExprMod(x->ty, xa->a, xab);
          return mutate_expr(x);
        }
      }
    }

    x = new ExprMod(
      mutate_ty(x->ty),
      mutate_expr(x->a),
      mutate_expr(x->b)
    );
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
