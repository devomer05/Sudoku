#include <iostream>
#include <vector>
#include <chrono>
#include <string>

#include "DatasetLoader.h"
#include "BacktrackingSolver.h"
#include "BacktrackingSolverMRV.h"
#include "LogicalSolver.h"
#include "LogicalSolverSIMD.h"
#include "ParallelSolver.h"

using Clock = std::chrono::high_resolution_clock;

/* ============================================================
   CONFIG
   ============================================================ */
static const bool RUN_SEQUENTIAL = true;
static const bool RUN_PARALLEL = false;
static const bool RUN_COMPARE = false;
static const size_t MAX_SUDOKU_PER_DATASET = 250;
static const int  THREAD_COUNT = 20;

/* ============================================================
   STATS PRINT
   ============================================================ */
static void printStats(
    const char* title,
    const SolveStats& stats,
    long long totalMs)
{
    size_t total = stats.total();

    std::cout << "[" << title << "] Total time: "
        << totalMs << " ms\n";
    std::cout << "[" << title << " Stats]\n";

    if (stats.alreadySolved)
        std::cout << "Already Solved         : "
        << stats.alreadySolved
        << " (" << 100.0 * stats.alreadySolved / total << "%)\n";

    if (stats.logical)
        std::cout << "Solved by Logical      : "
        << stats.logical
        << " (" << 100.0 * stats.logical / total << "%)\n";

    if (stats.backtracking)
        std::cout << "Solved by Backtracking : "
        << stats.backtracking
        << " (" << 100.0 * stats.backtracking / total << "%)\n";

    if (stats.unsolvable)
        std::cout << "Unsolvable             : "
        << stats.unsolvable << "\n";
}

/* ============================================================
   RUNNER
   ============================================================ */
static void runSolver(
    ISudokuSolver& solver,
    const std::vector<std::vector<Sudoku>>& baseDatasets,
    bool parallel,
    int threadCount,
    std::vector<std::vector<Sudoku>>* outCopy)
{
    // solver-local deep copy (AYNEN KALIR)
    std::vector<std::vector<Sudoku>> datasets = baseDatasets;

    SolveStats totalStats;
    Clock::time_point t0 = Clock::now();

    for (size_t d = 0; d < datasets.size(); ++d)
    {
        std::vector<Sudoku>& dataset = datasets[d];
        size_t clues = dataset.front().GetAssignedCellCount();
        size_t sudokuCount = dataset.size();

        Clock::time_point s = Clock::now();
        SolveStats ds;

        if (parallel)
            ds = ParallelSolver::solveAll(solver, dataset, threadCount);
        else
            ds = solver.solveAll(dataset);

        Clock::time_point e = Clock::now();

        long long elapsedMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count();

        double msPerSudoku =
            sudokuCount ? (double)elapsedMs / (double)sudokuCount : 0.0;

        totalStats.alreadySolved += ds.alreadySolved;
        totalStats.logical += ds.logical;
        totalStats.backtracking += ds.backtracking;
        totalStats.unsolvable += ds.unsolvable;

        std::cout
            << "[Dataset" << d << " ("
            << sudokuCount << " sudokus with "
            << clues << "/81 initial clues)] finished in "
            << elapsedMs << " ms "
            << "[" << msPerSudoku << " ms/sudoku]\n";
    }

    Clock::time_point t1 = Clock::now();
    printStats(
        (std::string(solver.getName()) +
            (parallel ? " Parallel" : " Sequential")).c_str(),
        totalStats,
        std::chrono::duration_cast<
        std::chrono::milliseconds>(t1 - t0).count());

    if (outCopy)
        *outCopy = datasets;

    // Logical stats (sadece destekleyen solverlar)
    LogicalSolver* ls = dynamic_cast<LogicalSolver*>(&solver);
    if (ls)
    {
        const LogicalStats& st = ls->getLogicalStats();
        std::cout << "\n[Logical Details]\n";
        std::cout << "NakedSingle        : hit=" << st.data[0][0]
            << " effect=" << st.data[0][1] << "\n";
        std::cout << "HiddenSingle       : hit=" << st.data[1][0]
            << " effect=" << st.data[1][1] << "\n";
        std::cout << "LockedPointing     : hit=" << st.data[2][0]
            << " effect=" << st.data[2][1] << "\n";
        std::cout << "LockedClaiming     : hit=" << st.data[3][0]
            << " effect=" << st.data[3][1] << "\n";
        std::cout << "NakedPair          : hit=" << st.data[4][0]
            << " effect=" << st.data[4][1] << "\n";
        std::cout << "HiddenPair         : hit=" << st.data[5][0]
            << " effect=" << st.data[5][1] << "\n";
    }
}

/* ============================================================
   MAIN
   ============================================================ */
int main()
{
    /*
    // Single Sudoku sanity test (AYNEN KALIR)

    Sudoku s;
    s.loadFromFile("hardest1.txt");
    LogicalSolver solver;
    solver.solve(s);
    std::cout << s << std::endl;
    return 0;
    */

    std::vector<std::vector<Sudoku>> baseDatasets =
        DatasetLoader::loadAllDatasets("Dataset");

    std::vector<std::vector<Sudoku>> groundTruth;
    std::vector<std::vector<Sudoku>> toTest;

    std::vector<ISudokuSolver*> solvers;
    //solvers.push_back(new BacktrackingSolver());
    //solvers.push_back(new BacktrackingSolverMRV());
    solvers.push_back(new LogicalSolver());
    //solvers.push_back(new LogicalSolverSIMD());

    for (size_t i = 0; i < solvers.size(); ++i)
    {
        ISudokuSolver& solver = *solvers[i];

        std::cout << "\n=== " << solver.getName() << " ===\n";

        if (RUN_SEQUENTIAL)
        {
            std::cout << "\n--- Sequential ---\n";
            runSolver(
                solver,
                baseDatasets,
                false,
                THREAD_COUNT,
                (RUN_COMPARE && i == 0) ? &groundTruth :
                (RUN_COMPARE ? &toTest : nullptr)
            );
        }

        if (RUN_PARALLEL)
        {
            std::cout << "\n--- Parallel ---\n";
            runSolver(
                solver,
                baseDatasets,
                true,
                THREAD_COUNT,
                nullptr
            );
        }
    }

    if (RUN_COMPARE && !groundTruth.empty() && !toTest.empty())
    {
        bool allSame = true;

        for (size_t d = 0; d < groundTruth.size(); ++d)
        {
            for (size_t i = 0; i < groundTruth[d].size(); ++i)
            {
                if (groundTruth[d][i] != toTest[d][i])
                {
                    std::cout << "[DIFF] Dataset "
                        << d << ", Index " << i << "\n";
                    allSame = false;
                    break;
                }
            }
            if (!allSame) break;
        }

        if (allSame)
            std::cout
            << "\n[COMPARE] All sequential solvers produced identical results.\n";
    }

    for (size_t i = 0; i < solvers.size(); ++i)
        delete solvers[i];

    return 0;
}
