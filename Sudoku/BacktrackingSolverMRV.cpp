#include "BacktrackingSolverMRV.h"

SolveResult BacktrackingSolverMRV::solve(Sudoku& sudoku)
{
    // Entry-point davranýþý BacktrackingSolver ile AYNI
    if (sudoku.isSolved())
        return SolveResult::SolvedByBacktracking;

    bool ok = solveRecursive(sudoku);
    return ok ? SolveResult::SolvedByBacktracking : SolveResult::Unsolvable;
}

bool BacktrackingSolverMRV::solveRecursive(Sudoku& sudoku)
{
    uint8_t row, col;

    // MRV ile hücre seçimi
    if (!sudoku.findCellWithMRV(row, col))
    {
        // Ya tamamen dolu (çözüm)
        // Ya da bu dal dead-end
        return sudoku.isSolved();
    }

    for (uint8_t num = 1; num <= NUMBER_COUNT; ++num)
    {
        if (sudoku.isSafe(row, col, num))
        {
            sudoku.set(row, col, num);

            if (solveRecursive(sudoku))
                return true;

            // geri al
            sudoku.set(row, col, UNASSIGNED);
        }
    }

    return false;
}