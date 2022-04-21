// Pass: Ranged-loop elevation
// @PENGUINLIONG
#include "visitor/visitor.hpp"

// Unwrap any sole member in block statements.
//
// ```
// StmtLoop
// +-StmtBlock
// | +-StmtBranchConditional
// |   +-StmtBlock
// |   | +-StmtLoopContinue
// |   +-StmtBlock
// |     +-StmtLoopMerge
// +-StmtBlock
//   +-StmtLoopMerge
// ```
//
// becomes:
//
// ```
// StmtLoop
// +-StmtBranchConditional
// | +-StmtLoopContinue
// | +-StmtLoopMerge
// +-StmtLoopMerge
// ```
extern void eliminate_forward_blocks(StmtRef& x);

// For any binary expression, if the expression has a constant operand and a
// non-constant operand, ensure the constant operand is always on the right-
// hand-side (`b`).
extern void expr_normalization(StmtRef& x);

extern void ranged_loop_elevation(StmtRef& x);
