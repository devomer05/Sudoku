#pragma once

#include "ISudokuSolver.h"
#include "Sudoku.h"

class BacktrackingSolverMRV : public ISudokuSolver
{
public:
    SolveResult solve(Sudoku& sudoku) override;
    const char* getName() const override { return "BacktrackingMRV Solver"; }
private:
    bool solveRecursive(Sudoku& sudoku);
};