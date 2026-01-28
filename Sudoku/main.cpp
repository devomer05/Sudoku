#include <iostream>
#include <vector>
#include <chrono>

#include "DatasetLoader.h"
#include "BacktrackingSolver.h"
#include "BacktrackingSolverMRV.h"
#include "LogicalSolver.h"

using Clock = std::chrono::high_resolution_clock;

int main()
{
    /*
    Sudoku s;
	s.loadFromFile("hardest1.txt");

	ISudokuSolver* solver = new BacktrackingSolver();
	SolveResult res = solver->solve(s);
    if(res == SolveResult::SolvedByBacktracking)
    {
        std::cout << "Solved by BacktrackingSolver:" << std::endl;
        std::cout << s << std::endl;
	}
    delete solver;

	solver = new BacktrackingSolverMRV();
    res = solver->solve(s);
    if (res == SolveResult::SolvedByBacktracking)
    {
        std::cout << "Solved by BacktrackingSolverMRV:" << std::endl;
        std::cout << s << std::endl;
    }
    delete solver;

    solver = new LogicalSolver();
    res = solver->solve(s);

    if(res == SolveResult::SolvedByLogical)
    {
        std::cout << "Solved by LogicalSolver:" << std::endl;
        std::cout << s << std::endl;
	}
    delete solver;

    return 0;
    */
    /* ============================================================
       DATASET YÜKLEME (1 KERE)
       ============================================================ */
    std::vector<std::vector<Sudoku>> datasets =
        DatasetLoader::loadAllDatasets("Dataset");

    /* ============================================================
       DATASET KOPYALAMA (BENCHMARK DIÞI)
       ============================================================ */
    //std::vector<std::vector<Sudoku>> datasetsForBacktracking = datasets;
    std::vector<std::vector<Sudoku>> datasetsForMRV = datasets;
    std::vector<std::vector<Sudoku>> datasetsForLogical = datasets;

    /* ============================================================
       1) KLASÝK BACKTRACKING
       ============================================================ */
    /*{
        std::cout << "\n=== BacktrackingSolver ===\n" << std::endl;

        BacktrackingSolver solver;
        Clock::time_point globalStart = Clock::now();

        for (size_t d = 0; d < datasetsForBacktracking.size(); ++d)
        {
            std::vector<Sudoku>& dataset = datasetsForBacktracking[d];

            std::cout << "[Dataset" << d << "] "
                << "count = " << dataset.size()
                << " ... solving..." << std::endl;

            Clock::time_point start = Clock::now();
            solver.solveAll(dataset);
            Clock::time_point end = Clock::now();

            long long ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    end - start).count();

            std::cout << "[Dataset" << d << "] "
                << "finished in " << ms << " ms"
                << std::endl;
        }

        Clock::time_point globalEnd = Clock::now();
        long long totalMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                globalEnd - globalStart).count();

        std::cout << "\n[BacktrackingSolver] All datasets finished in "
            << totalMs << " ms\n" << std::endl;
    }
    */
    /* ============================================================
       2) BACKTRACKING + MRV
       ============================================================ */
    {
        std::cout << "\n=== BacktrackingSolverMRV ===\n" << std::endl;

        BacktrackingSolverMRV solver;
        Clock::time_point globalStart = Clock::now();

        for (size_t d = 0; d < datasetsForMRV.size(); ++d)
        {
            std::vector<Sudoku>& dataset = datasetsForMRV[d];

            std::cout << "[Dataset" << d << "] "
                << "count = " << dataset.size()
                << " ... solving..." << std::endl;

            Clock::time_point start = Clock::now();
            solver.solveAll(dataset);
            Clock::time_point end = Clock::now();

            long long ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    end - start).count();

            std::cout << "[Dataset" << d << "] "
                << "finished in " << ms << " ms"
                << std::endl;
        }

        Clock::time_point globalEnd = Clock::now();
        long long totalMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                globalEnd - globalStart).count();

        std::cout << "\n[BacktrackingSolverMRV] All datasets finished in "
            << totalMs << " ms\n" << std::endl;
    }

    /* ============================================================
3) LOGICAL SOLVER
============================================================ */
    {
        std::cout << "\n=== LogicalSolver ===\n" << std::endl;
        size_t solvedByLogical = 0;
        size_t solvedByBacktracking = 0;
        size_t unsolvable = 0;

        LogicalSolver solver;
        Clock::time_point globalStart = Clock::now();


        for (size_t d = 0; d < datasetsForLogical.size(); ++d)
        {
            std::vector<Sudoku>& dataset = datasetsForLogical[d];


            std::cout << "[Dataset" << d << "] "
                << "count = " << dataset.size()
                << " ... solving..." << std::endl;


            Clock::time_point start = Clock::now();
            for (auto& s : dataset)
            {
                SolveResult res = solver.solve(s);

                if (res == SolveResult::SolvedByLogical)
                    ++solvedByLogical;
                else if (res == SolveResult::SolvedByBacktracking)
                    ++solvedByBacktracking;
                else
                    ++unsolvable;
            }
            Clock::time_point end = Clock::now();


            long long ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    end - start).count();


            std::cout << "[Dataset" << d << "] "
                << "finished in " << ms << " ms"
                << std::endl;
        }


        Clock::time_point globalEnd = Clock::now();
        long long totalMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                globalEnd - globalStart).count();

        size_t total =
            solvedByLogical + solvedByBacktracking + unsolvable;


        std::cout << "\n[LogicalSolver Stats]\n";
        std::cout << "Solved by Logical : "
            << solvedByLogical
            << " (" << (100.0 * solvedByLogical / total) << "%)\n";


        std::cout << "Solved by Backtracking: "
            << solvedByBacktracking
            << " (" << (100.0 * solvedByBacktracking / total) << "%)\n";


        if (unsolvable > 0)
        {
            std::cout << "Unsolvable : "
                << unsolvable << std::endl;
        }
        std::cout << "\n[LogicalSolver] All datasets finished in "
            << totalMs << " ms\n" << std::endl;
    }

    /* ============================================================
    KARÞILAÞTIRMA (BENCHMARK DIÞI)
    ============================================================ */
    {
        bool allSame = true;


        for (size_t d = 0; d < datasetsForLogical.size(); ++d)
        {
            const auto& logicalSet = datasetsForLogical[d];
            //const auto& backtrackSet = datasetsForBacktracking[d];
            const auto& mrvSet = datasetsForMRV[d];


            for (size_t i = 0; i < logicalSet.size(); ++i)
            {
                if (
                    !(logicalSet[i] == mrvSet[i]))
                {
                    allSame = false;


                    std::cout << "\n[DIFF] Dataset " << d
                        << ", Index " << i << std::endl;

                    std::cout << "BacktrackingMRV:\n";
                    std::cout << mrvSet[i] << std::endl;


                    std::cout << "LogicalSolver:\n";
                    std::cout << logicalSet[i] << std::endl;
                }
            }
        }

        if (allSame)
        {
            std::cout << "\n[COMPARE] All solvers produced identical results."
                << std::endl;
        }
    }
    return 0;
}
