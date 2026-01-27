#include <iostream>
#include <vector>
#include <chrono>

#include "DatasetLoader.h"
#include "BacktrackingSolver.h"
#include "BacktrackingSolverMRV.h"

using Clock = std::chrono::high_resolution_clock;

int main()
{
    /* ============================================================
       DATASET YÜKLEME (1 KERE)
       ============================================================ */
    std::vector<std::vector<Sudoku>> datasets =
        DatasetLoader::loadAllDatasets("Dataset");

    /* ============================================================
       DATASET KOPYALAMA (BENCHMARK DIÞI)
       ============================================================ */
    std::vector<std::vector<Sudoku>> datasetsForBacktracking = datasets;
    std::vector<std::vector<Sudoku>> datasetsForMRV = datasets;

    /* ============================================================
       1) KLASÝK BACKTRACKING
       ============================================================ */
    {
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

    return 0;
}

	/*Sudoku s;
   //s.loadFromFile("hardest1.txt");
   std::ifstream in("hardest1.txt");
   //std::cin >> s;
   in >> s;
   std::cout << s;

   if (s.validate())
   {
	   std::cout << "The initial Sudoku puzzle is valid." << std::endl;
   } else {
	   std::cout << "The initial Sudoku puzzle is invalid." << std::endl;
	   return 1;
   }
   BacktrackingSolver solver;
   SolveResult result = solver.solve(s);
   if (result == SolveResult::Solved)
   {
	   std::cout << "Sudoku solved successfully:" << std::endl;
	   std::cout << s << std::endl;
   }
   else if (result == SolveResult::Unsolvable)
   {
	   std::cout << "The Sudoku puzzle is unsolvable." << std::endl;
   }
   else
   {
	   std::cout << "The Sudoku puzzle was only partially solved." << std::endl;
   }
   */
