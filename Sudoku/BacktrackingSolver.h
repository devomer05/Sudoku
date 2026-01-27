#pragma once

#include "ISudokuSolver.h"
#include "Sudoku.h"

class BacktrackingSolver : public ISudokuSolver
{
public:
    BacktrackingSolver() = default;
    ~BacktrackingSolver() override = default;

    SolveResult solve(Sudoku& sudoku) override;

private:
    bool solveRecursive(Sudoku& sudoku);
};