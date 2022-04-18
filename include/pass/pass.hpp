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
extern StmtRef eliminate_forward_blocks(StmtRef& x);

extern StmtRef ranged_loop_elevation(StmtRef& x);
