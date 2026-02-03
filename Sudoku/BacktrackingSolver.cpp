#include "BacktrackingSolver.h"

SolveResult BacktrackingSolver::solve(Sudoku& sudoku)
{
    // Eðer zaten çözüldüyse
    if (sudoku.isSolved())
        return SolveResult::AlreadySolved;

    bool ok = solveRecursive(sudoku);
    return ok ? SolveResult::SolvedByBacktracking : SolveResult::Unsolvable;
}

bool BacktrackingSolver::solveRecursive(Sudoku& sudoku)
{
    uint8_t row, col;

    // Boþ hücre yoksa çözülmüþtür
    if (!sudoku.findUnassigned(row, col))
        return true;

    for (uint8_t num = 1; num <= NUMBER_COUNT; ++num)
    {
        if (sudoku.isSafe(row, col, num))
        {
            sudoku.set(row, col, num);

            if (solveRecursive(sudoku))
                return true;

            // Geri al
            sudoku.set(row, col, UNASSIGNED);
        }
    }
    return false;
}