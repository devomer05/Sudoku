#pragma once
#include "ISudokuSolver.h"
#include "Sudoku.h"

struct LogicalStats {
	// [technique][metric]
	// metric: 0 = hit, 1 = effect
	uint32_t data[8][2] = {};
};

enum {
	LS_NAKED_SINGLE = 0,
	LS_HIDDEN_SINGLE,
	LS_LOCKED_POINTING,
	LS_LOCKED_CLAIMING,
	LS_NAKED_PAIR,
	LS_HIDDEN_PAIR,
	LS_NAKED_TRIPLE,
	LS_HIDDEN_TRIPLE,
};


class LogicalSolver : public ISudokuSolver
{
	
protected:
	virtual bool applyNakedSingle(Sudoku& s);
	bool applyHiddenSingle(Sudoku& s);
	bool applyLockedCandidatesPointing(Sudoku& sudoku);
	bool applyLockedCandidatesClaiming(Sudoku& sudoku);
	bool applyNakedPair(Sudoku& s);
	bool applyHiddenPair(Sudoku& s);
	bool applyLogicalStep(Sudoku& s);
	bool applyNakedTriple(Sudoku& s);
	bool applyHiddenTriple(Sudoku& s);
	LogicalStats logicalStats;
public:
	const LogicalStats& getLogicalStats() const { return logicalStats; }
	const char* getName() const override { return "Logical Solver"; }
	SolveResult solve(Sudoku& s);
};