#pragma once
#include "ISudokuSolver.h"
#include "Sudoku.h"

class LogicalSolver : public ISudokuSolver
{
private:
	bool applyNakedSingle(Sudoku& s);
	bool applyHiddenSingle(Sudoku& s);
	bool applyLockedCandidatesPointing(Sudoku& sudoku);
	bool applyLockedCandidatesClaiming(Sudoku& sudoku);
	bool applyNakedPair(Sudoku& s);
	bool applyHiddenPair(Sudoku& s);
	bool applyLogicalStep(Sudoku& s);

public:
	const char* getName() const override { return "Logical Solver"; }
	SolveResult solve(Sudoku& s);
};