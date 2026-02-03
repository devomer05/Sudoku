#pragma once

#include "ISudokuSolver.h"
#include "Sudoku.h"

class BacktrackingSolver : public ISudokuSolver
{
public:
    BacktrackingSolver() = default;
    ~BacktrackingSolver() override = default;

    SolveResult solve(Sudoku& sudoku) override;
    const char* getName() const override { return "Backtracking Solver"; }
private:
    bool solveRecursive(Sudoku& sudoku);
};