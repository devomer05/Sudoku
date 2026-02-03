#include <iostream>
#include <vector>
#include <chrono>

#include "DatasetLoader.h"
#include "BacktrackingSolverMRV.h"
#include "LogicalSolver.h"
#include "ParallelSolver.h"

using Clock = std::chrono::high_resolution_clock;

int main()
{
    /*
    // Single Sudoku sanity test (manual debugging)

    Sudoku s;
    s.loadFromFile("hardest1.txt");

    ISudokuSolver* solver = new BacktrackingSolver();
    SolveResult res = solver->solve(s);
    if (res == SolveResult::SolvedByBacktracking)
    {
        std::cout << "Solved by BacktrackingSolver:\n";
        std::cout << s << std::endl;
    }
    delete solver;

    solver = new BacktrackingSolverMRV();
    res = solver->solve(s);
    if (res == SolveResult::SolvedByBacktracking)
    {
        std::cout << "Solved by BacktrackingSolverMRV:\n";
        std::cout << s << std::endl;
    }
    delete solver;

    solver = new LogicalSolver();
    res = solver->solve(s);
    if (res == SolveResult::SolvedByLogical)
    {
        std::cout << "Solved by LogicalSolver:\n";
        std::cout << s << std::endl;
    }
    delete solver;

    return 0;
    */

    /* ============================================================
       LOAD DATASETS (ONCE)
       ============================================================ */
    std::vector<std::vector<Sudoku>> baseDatasets =
        DatasetLoader::loadAllDatasets("Dataset");

    /* ============================================================
       SOLVERS
       ============================================================ */
    BacktrackingSolverMRV solverMRV;
    LogicalSolver solverLogical;

    ISudokuSolver* solvers[2];
    solvers[0] = &solverMRV;
    solvers[1] = &solverLogical;

    /* ============================================================
       DATASET COPIES (ISOLATED PER SOLVER / MODE)
       ============================================================ */
    std::vector<std::vector<Sudoku>> sequentialDatasets[2];
    std::vector<std::vector<Sudoku>> parallelDatasets[2];

    sequentialDatasets[0] = baseDatasets;
    sequentialDatasets[1] = baseDatasets;

    parallelDatasets[0] = baseDatasets;
    parallelDatasets[1] = baseDatasets;

    /* ============================================================
       BENCHMARK
       ============================================================ */
    for (int solverIndex = 0; solverIndex < 2; ++solverIndex)
    {
        ISudokuSolver* solver = solvers[solverIndex];

        /* ----------------------------
           SEQUENTIAL RUN
           ---------------------------- */
        std::cout << "\n=== " << solver->getName()
            << " (Sequential) ===\n";

        SolveStats sequentialStats;
        Clock::time_point sequentialStart = Clock::now();

        for (size_t datasetIndex = 0;
            datasetIndex < sequentialDatasets[solverIndex].size();
            ++datasetIndex)
        {
            std::vector<Sudoku>& dataset =
                sequentialDatasets[solverIndex][datasetIndex];

            uint8_t initialClues =
                dataset[0].GetAssignedCellCount();

            std::cout << "[Dataset" << datasetIndex << "] "
                << "count = " << dataset.size()
                << " | initial clues = "
                << (int)initialClues << "/81"
                << " ... solving...\n";

            Clock::time_point start = Clock::now();
            SolveStats stats = solver->solveAll(dataset);
            Clock::time_point end = Clock::now();

            sequentialStats.logical += stats.logical;
            sequentialStats.backtracking += stats.backtracking;
            sequentialStats.unsolvable += stats.unsolvable;

            long long elapsedMs =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    end - start).count();

            std::cout << "[Dataset" << datasetIndex
                << "] finished in "
                << elapsedMs << " ms\n";
        }

        Clock::time_point sequentialEnd = Clock::now();
        long long sequentialTotalMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                sequentialEnd - sequentialStart).count();

        std::cout << "[" << solver->getName()
            << " Sequential] Total time: "
            << sequentialTotalMs << " ms\n";
        size_t sequentialTotal =
            sequentialStats.logical +
            sequentialStats.backtracking +
            sequentialStats.unsolvable;

        std::cout << "\n[" << solver->getName()
            << " Sequential Stats]\n";

        if (sequentialStats.logical > 0)
        {
            std::cout << "Solved by Logical      : "
                << sequentialStats.logical
                << " ("
                << (100.0 * sequentialStats.logical / sequentialTotal)
                << "%)\n";
        }

        if (sequentialStats.backtracking > 0)
        {
            std::cout << "Solved by Backtracking : "
                << sequentialStats.backtracking
                << " ("
                << (100.0 * sequentialStats.backtracking / sequentialTotal)
                << "%)\n";
        }

        if (sequentialStats.unsolvable > 0)
        {
            std::cout << "Unsolvable             : "
                << sequentialStats.unsolvable << "\n";
        }
        /* ----------------------------
           PARALLEL RUN
           ---------------------------- */
        std::cout << "\n=== " << solver->getName()
            << " (Parallel) ===\n";

        SolveStats parallelStats;
        Clock::time_point parallelStart = Clock::now();

        for (size_t datasetIndex = 0;
            datasetIndex < parallelDatasets[solverIndex].size();
            ++datasetIndex)
        {
            std::vector<Sudoku>& dataset =
                parallelDatasets[solverIndex][datasetIndex];

            Clock::time_point start = Clock::now();
            SolveStats stats =
                ParallelSolver::solveAll(*solver, dataset);
            Clock::time_point end = Clock::now();

            parallelStats.logical += stats.logical;
            parallelStats.backtracking += stats.backtracking;
            parallelStats.unsolvable += stats.unsolvable;

            long long elapsedMs =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    end - start).count();

            std::cout << "[Dataset" << datasetIndex
                << "] finished in "
                << elapsedMs << " ms\n";
        }

        Clock::time_point parallelEnd = Clock::now();
        long long parallelTotalMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                parallelEnd - parallelStart).count();

        std::cout << "[" << solver->getName()
            << " Parallel] Total time: "
            << parallelTotalMs << " ms\n";

        /* ----------------------------
           PARALLEL STATS REPORT
           ---------------------------- */
        size_t totalSolved =
            parallelStats.logical +
            parallelStats.backtracking +
            parallelStats.unsolvable;

        std::cout << "\n[" << solver->getName()
            << " Parallel Stats]\n";

        if (parallelStats.logical > 0)
        {
            std::cout << "Solved by Logical      : "
                << parallelStats.logical
                << " ("
                << (100.0 * parallelStats.logical / totalSolved)
                << "%)\n";
        }

        if (parallelStats.backtracking > 0)
        {
            std::cout << "Solved by Backtracking : "
                << parallelStats.backtracking
                << " ("
                << (100.0 * parallelStats.backtracking / totalSolved)
                << "%)\n";
        }

        if (parallelStats.unsolvable > 0)
        {
            std::cout << "Unsolvable             : "
                << parallelStats.unsolvable << "\n";
        }
    }

    /* ============================================================
       RESULT COMPARISON (SEQUENTIAL ONLY)
       ============================================================ */
    {
        bool allResultsMatch = true;

        std::vector<std::vector<Sudoku>>& mrvResults =
            sequentialDatasets[0];
        std::vector<std::vector<Sudoku>>& logicalResults =
            sequentialDatasets[1];

        for (size_t d = 0; d < mrvResults.size(); ++d)
        {
            for (size_t i = 0; i < mrvResults[d].size(); ++i)
            {
                if (!(mrvResults[d][i] == logicalResults[d][i]))
                {
                    allResultsMatch = false;

                    std::cout << "\n[DIFF] Dataset "
                        << d << ", Index " << i << "\n";

                    std::cout << "BacktrackingMRV:\n"
                        << mrvResults[d][i] << "\n";

                    std::cout << "LogicalSolver:\n"
                        << logicalResults[d][i] << "\n";
                }
            }
        }

        if (allResultsMatch)
        {
            std::cout
                << "\n[COMPARE] All sequential solvers produced identical results.\n";
        }
    }

    return 0;
}
