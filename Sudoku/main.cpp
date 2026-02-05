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
    // Single Sudoku sanity test (manual debugging)

    Sudoku s;
    s.loadFromFile("hardest1.txt");

    LogicalSolver solver;
    solver.solve(s);
    std::cout << s << std::endl;

    return 0;
    */

    /* ============================================================
       LOAD DATASETS (ONCE)
       ============================================================ */
    std::vector<std::vector<Sudoku>> baseDatasets =
        DatasetLoader::loadAllDatasets("Dataset");

    /* ============================================================
       PREPARE DATASETS (FULL COPIES)
       ============================================================ */
    std::vector<std::vector<Sudoku>> mrvSeqDatasets = baseDatasets;
    std::vector<std::vector<Sudoku>> mrvParDatasets = baseDatasets;
    std::vector<std::vector<Sudoku>> logicalSeqDatasets = baseDatasets;
    std::vector<std::vector<Sudoku>> logicalParDatasets = baseDatasets;

    /* ============================================================
       SOLVERS
       ============================================================ */
    BacktrackingSolverMRV mrvSolver;
    LogicalSolver logicalSolver;

    ISudokuSolver* solvers[2] = {
        &mrvSolver,
        &logicalSolver
    };

    std::vector<std::vector<Sudoku>>* seqDatasets[2] = {
        &mrvSeqDatasets,
        &logicalSeqDatasets
    };

    std::vector<std::vector<Sudoku>>* parDatasets[2] = {
        &mrvParDatasets,
        &logicalParDatasets
    };

    const int threadCount = 10;

    /* ============================================================
       BENCHMARK (SINGLE SOLVER LOOP)
       ============================================================ */
    for (int i = 0; i < 2; ++i)
    {
        ISudokuSolver& solver = *solvers[i];

        /* ----------------------------
           SEQUENTIAL
           ---------------------------- */
        {
            std::cout << "\n=== " << solver.getName()
                << " (Sequential) ===\n";

            SolveStats stats;
            Clock::time_point t0 = Clock::now();

            for (size_t d = 0; d < seqDatasets[i]->size(); ++d)
            {
                auto& dataset = (*seqDatasets[i])[d];

                size_t clues =  dataset.front().GetAssignedCellCount();

                Clock::time_point s = Clock::now();
                SolveStats ds = solver.solveAll(dataset);
                Clock::time_point e = Clock::now();

                stats.alreadySolved += ds.alreadySolved;
                stats.logical += ds.logical;
                stats.backtracking += ds.backtracking;
                stats.unsolvable += ds.unsolvable;

                std::cout
                    << "[Dataset" << d << " ("
                    << clues << "/81 initial clues)] finished in "
                    << std::chrono::duration_cast<
                    std::chrono::milliseconds>(e - s).count()
                    << " ms\n";
            }

            Clock::time_point t1 = Clock::now();
            printStats(
                (std::string(solver.getName()) + " Sequential").c_str(),
                stats,
                std::chrono::duration_cast<
                std::chrono::milliseconds>(t1 - t0).count());
        }

        /* ----------------------------
           PARALLEL
           ---------------------------- */
        {
            std::cout << "\n=== " << solver.getName()
                << " (Parallel) ===\n";

            SolveStats stats;
            Clock::time_point t0 = Clock::now();

            for (size_t d = 0; d < parDatasets[i]->size(); ++d)
            {
                auto& dataset = (*parDatasets[i])[d];
                size_t clues = dataset.front().GetAssignedCellCount();

                Clock::time_point s = Clock::now();
                SolveStats ds =
                    ParallelSolver::solveAll(
                        solver,
                        (*parDatasets[i])[d],
                        threadCount);
                Clock::time_point e = Clock::now();

                stats.alreadySolved += ds.alreadySolved;
                stats.logical += ds.logical;
                stats.backtracking += ds.backtracking;
                stats.unsolvable += ds.unsolvable;

                std::cout
                    << "[Dataset" << d << " ("
                    << clues << "/81 initial clues)] finished in "
                    << std::chrono::duration_cast<
                    std::chrono::milliseconds>(e - s).count()
                    << " ms\n";
            }

            Clock::time_point t1 = Clock::now();
            printStats(
                (std::string(solver.getName()) + " Parallel").c_str(),
                stats,
                std::chrono::duration_cast<
                std::chrono::milliseconds>(t1 - t0).count());
        }
    }

    /* ============================================================
       RESULT COMPARISON (SEQUENTIAL ONLY)
       ============================================================ */
    {
        bool allSame = true;

        for (size_t d = 0; d < mrvSeqDatasets.size(); ++d)
        {
            for (size_t i = 0; i < mrvSeqDatasets[d].size(); ++i)
            {
                if (!(mrvSeqDatasets[d][i] == logicalSeqDatasets[d][i]))
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
        {
            std::cout
                << "\n[COMPARE] All sequential solvers produced identical results.\n";
        }
    }

    return 0;
}
