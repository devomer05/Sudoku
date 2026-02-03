#include "ParallelSolver.h"
#include <iostream>

SolveStats ParallelSolver::solveAll(
    ISudokuSolver& solver,
    std::vector<Sudoku>& sudokus,
    unsigned threadCount)
{
    const unsigned hw = std::thread::hardware_concurrency();
    const unsigned threads =
        std::max(1u, std::min(threadCount, hw ? hw : 1u));
    std::cout << "[INFO] Parallel threads count = " << threads << "\n";
    std::atomic<size_t> index{ 0 };
    std::atomic<size_t> logical{ 0 };
    std::atomic<size_t> backtracking{ 0 };
    std::atomic<size_t> unsolvable{ 0 };

    std::vector<std::thread> pool;
    pool.reserve(threads);

    for (unsigned t = 0; t < threads; ++t)
    {
        pool.emplace_back([&]()
            {
                while (true)
                {
                    size_t i = index.fetch_add(1, std::memory_order_relaxed);
                    if (i >= sudokus.size())
                        break;

                    SolveResult r = solver.solve(sudokus[i]);

                    if (r == SolveResult::SolvedByLogical)
                        ++logical;
                    else if (r == SolveResult::SolvedByBacktracking)
                        ++backtracking;
                    else
                        ++unsolvable;
                }
            });
    }

    for (auto& th : pool)
        th.join();

    SolveStats stats;
    stats.logical = logical.load();
    stats.backtracking = backtracking.load();
    stats.unsolvable = unsolvable.load();
    return stats;
}
