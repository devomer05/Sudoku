#pragma once
#include <vector>
#include "Sudoku.h"
class Sudoku;

enum class SolveResult
{
    AlreadySolved,
    SolvedByBacktracking,      // Tam çözüldü - backtracking
	SolvedByLogical,    // Tam çözüldü - mantýksal yöntemler
    Unsolvable,  // Çözüm yok
};

struct SolveStats {
	size_t alreadySolved = 0;
    size_t logical = 0;
    size_t backtracking = 0;
    size_t unsolvable = 0;
    size_t total() const {
        return alreadySolved + logical + backtracking + unsolvable;
	}
};

class ISudokuSolver
{
public:
    virtual ~ISudokuSolver() = default;

    // Tek bir Sudoku çöz
    virtual SolveResult solve(Sudoku& sudoku) = 0;

    // Çoklu Sudoku çözümü (batch / CUDA / MT yolu)
    virtual SolveStats solveAll(std::vector<Sudoku>& sudokus)
    {
        SolveStats stats;
        for (auto& s : sudokus)
        {
            SolveResult r = solve(s);
			if (r == SolveResult::AlreadySolved) ++stats.alreadySolved;
            else if (r == SolveResult::SolvedByLogical) ++stats.logical;
            else if (r == SolveResult::SolvedByBacktracking) ++stats.backtracking;
            else ++stats.unsolvable;
        }
        return stats;
    }
    virtual const char* getName() const = 0;
protected:
    ISudokuSolver() = default;
};