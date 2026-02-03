#include <iostream>
#include <vector>
#include <chrono>

#include "DatasetLoader.h"
#include "BacktrackingSolverMRV.h"
#include "LogicalSolver.h"
#include "ParallelSolver.h"

using Clock = std::chrono::high_resolution_clock;

static void printStats(
    const char* title,
    const SolveStats& stats,
    long long totalMs)
{
    size_t total = stats.total();

    std::cout << "[" << title << "] Total time: "
        << totalMs << " ms\n";
    std::cout << "[" << title << " Stats]\n";

    if (stats.alreadySolved > 0)
        std::cout << "Already Solved         : "
        << stats.alreadySolved
        << " (" << 100.0 * stats.alreadySolved / total << "%)\n";

    if (stats.logical > 0)
        std::cout << "Solved by Logical      : "
        << stats.logical
        << " (" << 100.0 * stats.logical / total << "%)\n";

    if (stats.backtracking > 0)
        std::cout << "Solved by Backtracking : "
        << stats.backtracking
        << " (" << 100.0 * stats.backtracking / total << "%)\n";

    if (stats.unsolvable > 0)
        std::cout << "Unsolvable             : "
        << stats.unsolvable << "\n";
}

int main()
{
    /*
    // Single Sudoku sanity test
    Sudoku s;
    s.loadFromFile("hardest1.txt");
    LogicalSolver solver;
    solver.solve(s);
    std::cout << s << std::endl;
    return 0;
    */

    /* ============================================================
       LOAD DATASETS ONCE
       ============================================================ */
    std::vector<std::vector<Sudoku>> baseDatasets =
        DatasetLoader::loadAllDatasets("Dataset");

    /* ============================================================
       PREPARE ALL DATASETS (FULL COPIES)
       ============================================================ */
    std::vector<std::vector<Sudoku>> mrvSeq = baseDatasets;
    std::vector<std::vector<Sudoku>> mrvPar = baseDatasets;
    std::vector<std::vector<Sudoku>> logicalSeq = baseDatasets;
    std::vector<std::vector<Sudoku>> logicalPar = baseDatasets;

    /* ============================================================
       SOLVERS
       ============================================================ */
    BacktrackingSolverMRV mrvSolver;
    LogicalSolver logicalSolver;

    /* ============================================================
       BACKTRACKING MRV - SEQUENTIAL
       ============================================================ */
    {
        std::cout << "\n=== " << mrvSolver.getName()
            << " (Sequential) ===\n";

        SolveStats stats;
        Clock::time_point t0 = Clock::now();

        for (size_t d = 0; d < mrvSeq.size(); ++d)
        {
            Clock::time_point s = Clock::now();
            SolveStats ds = mrvSolver.solveAll(mrvSeq[d]);
            Clock::time_point e = Clock::now();

            stats.alreadySolved += ds.alreadySolved;
            stats.logical += ds.logical;
            stats.backtracking += ds.backtracking;
            stats.unsolvable += ds.unsolvable;

            std::cout << "[Dataset" << d << "] finished in "
                << std::chrono::duration_cast<
                std::chrono::milliseconds>(e - s).count()
                << " ms\n";
        }

        Clock::time_point t1 = Clock::now();
        printStats(
            "BacktrackingMRV Sequential",
            stats,
            std::chrono::duration_cast<
            std::chrono::milliseconds>(t1 - t0).count());
    }

    /* ============================================================
       BACKTRACKING MRV - PARALLEL
       ============================================================ */
    {
        std::cout << "\n=== " << mrvSolver.getName()
            << " (Parallel) ===\n";

        SolveStats stats;
        Clock::time_point t0 = Clock::now();

        for (size_t d = 0; d < mrvPar.size(); ++d)
        {
            Clock::time_point s = Clock::now();
            SolveStats ds =
                ParallelSolver::solveAll(mrvSolver, mrvPar[d], 10);
            Clock::time_point e = Clock::now();

            stats.alreadySolved += ds.alreadySolved;
            stats.logical += ds.logical;
            stats.backtracking += ds.backtracking;
            stats.unsolvable += ds.unsolvable;

            std::cout << "[Dataset" << d << "] finished in "
                << std::chrono::duration_cast<
                std::chrono::milliseconds>(e - s).count()
                << " ms\n";
        }

        Clock::time_point t1 = Clock::now();
        printStats(
            "BacktrackingMRV Parallel",
            stats,
            std::chrono::duration_cast<
            std::chrono::milliseconds>(t1 - t0).count());
    }

    /* ============================================================
       LOGICAL SOLVER - SEQUENTIAL
       ============================================================ */
    {
        std::cout << "\n=== " << logicalSolver.getName()
            << " (Sequential) ===\n";

        SolveStats stats;
        Clock::time_point t0 = Clock::now();

        for (size_t d = 0; d < logicalSeq.size(); ++d)
        {
            Clock::time_point s = Clock::now();
            SolveStats ds = logicalSolver.solveAll(logicalSeq[d]);
            Clock::time_point e = Clock::now();

            stats.alreadySolved += ds.alreadySolved;
            stats.logical += ds.logical;
            stats.backtracking += ds.backtracking;
            stats.unsolvable += ds.unsolvable;

            std::cout << "[Dataset" << d << "] finished in "
                << std::chrono::duration_cast<
                std::chrono::milliseconds>(e - s).count()
                << " ms\n";
        }

        Clock::time_point t1 = Clock::now();
        printStats(
            "LogicalSolver Sequential",
            stats,
            std::chrono::duration_cast<
            std::chrono::milliseconds>(t1 - t0).count());
    }

    /* ============================================================
       LOGICAL SOLVER - PARALLEL
       ============================================================ */
    {
        std::cout << "\n=== " << logicalSolver.getName()
            << " (Parallel) ===\n";

        SolveStats stats;
        Clock::time_point t0 = Clock::now();

        for (size_t d = 0; d < logicalPar.size(); ++d)
        {
            Clock::time_point s = Clock::now();
            SolveStats ds =
                ParallelSolver::solveAll(logicalSolver, logicalPar[d], 10);
            Clock::time_point e = Clock::now();

            stats.alreadySolved += ds.alreadySolved;
            stats.logical += ds.logical;
            stats.backtracking += ds.backtracking;
            stats.unsolvable += ds.unsolvable;

            std::cout << "[Dataset" << d << "] finished in "
                << std::chrono::duration_cast<
                std::chrono::milliseconds>(e - s).count()
                << " ms\n";
        }

        Clock::time_point t1 = Clock::now();
        printStats(
            "LogicalSolver Parallel",
            stats,
            std::chrono::duration_cast<
            std::chrono::milliseconds>(t1 - t0).count());
    }

    /* ============================================================
       RESULT COMPARISON (SEQUENTIAL ONLY)
       ============================================================ */
    {
        bool allSame = true;

        for (size_t d = 0; d < mrvSeq.size(); ++d)
        {
            for (size_t i = 0; i < mrvSeq[d].size(); ++i)
            {
                if (!(mrvSeq[d][i] == logicalSeq[d][i]))
                {
                    std::cout << "\n[DIFF] Dataset "
                        << d << ", Index " << i << "\n";
                    allSame = false;
                    break;
                }
            }
            if (!allSame)
                break;
        }

        if (allSame)
            std::cout << "\n[COMPARE] All sequential solvers produced identical results.\n";
    }

    return 0;
}
