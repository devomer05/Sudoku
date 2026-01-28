#pragma once
#include "ISudokuSolver.h"
#include "Sudoku.h"

class LogicalSolver : public ISudokuSolver
{
public:
	bool applyNakedSingle(Sudoku& s);
	bool applyHiddenSingle(Sudoku& s);
	bool applyLockedCandidatesPointing(Sudoku& sudoku);
	bool applyLockedCandidatesClaiming(Sudoku& sudoku);
	bool applyNakedPair(Sudoku& s);
	bool applyHiddenPair(Sudoku& s);

	bool applyLogicalStep(Sudoku& s);

	SolveResult solve(Sudoku& s);
};