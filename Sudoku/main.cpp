#include <iostream>
#include <vector>
#include <chrono>

#include "DatasetLoader.h"
#include "BacktrackingSolver.h"
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
    std::vector<std::vector<Sudoku>> datasets =
        DatasetLoader::loadAllDatasets("Dataset");

    /* ============================================================
       DATASET COPIES (OUTSIDE BENCHMARK)
       ============================================================ */
    std::vector<std::vector<Sudoku>> datasetsMRV_seq = datasets;
    std::vector<std::vector<Sudoku>> datasetsLogical_seq = datasets;

    std::vector<std::vector<Sudoku>> datasetsMRV_par = datasets;
    std::vector<std::vector<Sudoku>> datasetsLogical_par = datasets;

    /* ============================================================
       SOLVER LIST (SEQUENTIAL + PARALLEL)
       ============================================================ */
    BacktrackingSolverMRV solverMRV;
    LogicalSolver solverLogical;

    std::vector<ISudokuSolver*> solvers;
    solvers.push_back(&solverMRV);
    solvers.push_back(&solverLogical);

    std::vector<std::vector<std::vector<Sudoku>>*> seqDatasets;
    seqDatasets.push_back(&datasetsMRV_seq);
    seqDatasets.push_back(&datasetsLogical_seq);

    std::vector<std::vector<std::vector<Sudoku>>*> parDatasets;
    parDatasets.push_back(&datasetsMRV_par);
    parDatasets.push_back(&datasetsLogical_par);

    /* ============================================================
       BENCHMARK: SEQUENTIAL + PARALLEL IN SAME LOOP
       ============================================================ */
    for (size_t s = 0; s < solvers.size(); ++s)
    {
        ISudokuSolver* solver = solvers[s];

        std::vector<std::vector<Sudoku>>& seqData =
            *seqDatasets[s];
        std::vector<std::vector<Sudoku>>& parData =
            *parDatasets[s];

        /* ----------------------------
           SEQUENTIAL
           ---------------------------- */
        std::cout << "\n=== " << solver->getName()
            << " (Sequential) ===\n";

        Clock::time_point seqStart = Clock::now();

        for (size_t d = 0; d < seqData.size(); ++d)
        {
            std::vector<Sudoku>& dataset = seqData[d];
            uint8_t clues = dataset[0].GetAssignedCellCount();

            std::cout << "[Dataset" << d << "] "
                << "count = " << dataset.size()
                << " | initial clues = " << (int)clues << "/81"
                << " ... solving...\n";

            Clock::time_point start = Clock::now();
            solver->solveAll(dataset);
            Clock::time_point end = Clock::now();

            long long ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    end - start).count();

            std::cout << "[Dataset" << d << "] finished in "
                << ms << " ms\n";
        }

        Clock::time_point seqEnd = Clock::now();
        long long seqTotalMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                seqEnd - seqStart).count();

        std::cout << "[" << solver->getName()
            << " Sequential] Total time: "
            << seqTotalMs << " ms\n";

        /* ----------------------------
           PARALLEL
           ---------------------------- */
        std::cout << "\n=== " << solver->getName()
            << " (Parallel) ===\n";

        Clock::time_point parStart = Clock::now();

        for (size_t d = 0; d < parData.size(); ++d)
        {
            std::vector<Sudoku>& dataset = parData[d];

            Clock::time_point start = Clock::now();
            ParallelSolver::solveAll(*solver, dataset);
            Clock::time_point end = Clock::now();

            long long ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    end - start).count();

            std::cout << "[Dataset" << d << "] finished in "
                << ms << " ms\n";
        }

        Clock::time_point parEnd = Clock::now();
        long long parTotalMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                parEnd - parStart).count();

        std::cout << "[" << solver->getName()
            << " Parallel] Total time: "
            << parTotalMs << " ms\n";
    }

    /* ============================================================
       RESULT COMPARISON (SEQUENTIAL RESULTS ONLY)
       ============================================================ */
    {
        bool allSame = true;

        for (size_t d = 0; d < datasetsLogical_seq.size(); ++d)
        {
            const std::vector<Sudoku>& logicalSet =
                datasetsLogical_seq[d];
            const std::vector<Sudoku>& mrvSet =
                datasetsMRV_seq[d];

            for (size_t i = 0; i < logicalSet.size(); ++i)
            {
                if (!(logicalSet[i] == mrvSet[i]))
                {
                    allSame = false;

                    std::cout << "\n[DIFF] Dataset "
                        << d << ", Index " << i << "\n";

                    std::cout << "BacktrackingMRV:\n"
                        << mrvSet[i] << "\n";

                    std::cout << "LogicalSolver:\n"
                        << logicalSet[i] << "\n";
                }
            }
        }

        if (allSame)
        {
            std::cout
                << "\n[COMPARE] All sequential solvers produced identical results.\n";
        }
    }

    return 0;
}
