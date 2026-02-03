#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include "ISudokuSolver.h"

class ParallelSolver
{
public:
    static SolveStats solveAll(
        ISudokuSolver& solver,
        std::vector<Sudoku>& sudokus,
        unsigned threadCount = std::thread::hardware_concurrency());
};
