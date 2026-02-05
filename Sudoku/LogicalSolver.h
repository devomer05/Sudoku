#pragma once
#include "ISudokuSolver.h"
#include "Sudoku.h"

struct LogicalStats {
	// [technique][metric]
	// metric: 0 = hit, 1 = effect
	uint32_t data[6][2] = {};
};

enum {
	LS_NAKED_SINGLE = 0,
	LS_HIDDEN_SINGLE,
	LS_LOCKED_POINTING,
	LS_LOCKED_CLAIMING,
	LS_NAKED_PAIR,
	LS_HIDDEN_PAIR
};


class LogicalSolver : public ISudokuSolver
{
private:
	
	bool applyHiddenSingle(Sudoku& s);
	bool applyLockedCandidatesPointing(Sudoku& sudoku);
	bool applyLockedCandidatesClaiming(Sudoku& sudoku);
	bool applyNakedPair(Sudoku& s);
	bool applyHiddenPair(Sudoku& s);
	bool applyLogicalStep(Sudoku& s);
protected:
	virtual bool applyNakedSingle(Sudoku& s);
	LogicalStats logicalStats;
public:
	const LogicalStats& getLogicalStats() const { return logicalStats; }
	const char* getName() const override { return "Logical Solver"; }
	SolveResult solve(Sudoku& s);
};