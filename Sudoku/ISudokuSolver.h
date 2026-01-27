#pragma once
#include <vector>
#include "Sudoku.h"
class Sudoku;

enum class SolveResult
{
    Solved,      // Tam çözüldü
    Unsolvable,  // Çözüm yok
    Partial      // Kýsmen ilerledi (logical solver için)
};

class ISudokuSolver
{
public:
    virtual ~ISudokuSolver() = default;

    // Tek bir Sudoku çöz
    virtual SolveResult solve(Sudoku& sudoku) = 0;

    // Çoklu Sudoku çözümü (batch / CUDA / MT yolu)
    virtual void solveAll(std::vector<Sudoku>& sudokus)
    {
        for (auto& s : sudokus)
            solve(s);
    }

protected:
    ISudokuSolver() = default;
};