#pragma once

#include "ISudokuSolver.h"
#include "Sudoku.h"

class BacktrackingSolverMRV : public ISudokuSolver
{
public:
    SolveResult solve(Sudoku& sudoku) override;

private:
    bool solveRecursive(Sudoku& sudoku);
};