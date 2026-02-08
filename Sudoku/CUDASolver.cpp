#include "CUDASolver.h"
#include "Sudoku.h"
#include "LogicalSolver.h"

#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <chrono>

extern "C" void runCudaLogicalBasic(
    uint8_t* grids,
    uint16_t* cands,
    int sudokuCount);

using Clock = std::chrono::high_resolution_clock;

CUDASolver::CUDASolver() {}
CUDASolver::~CUDASolver() {}

void CUDASolver::solve(std::vector<std::vector<Sudoku>>& datasets)
{
    
    std::vector<Sudoku*> flat;
    flat.reserve(120000);

    for (std::vector<Sudoku>& ds : datasets)
        for (Sudoku& s : ds)
            flat.push_back(&s);

    const int count = (int)flat.size();
    if (count == 0) return;

    std::vector<uint8_t> grids(count * 81);
    std::vector<uint16_t> cands(count * 81);

    // === EXTRACT (memcpy) ===
    for (int i = 0; i < count; ++i)
    {
        Sudoku* s = flat[i];

        std::memcpy(
            &grids[i * 81],
            s->rawGrid(),
            81);

        std::memcpy(
            &cands[i * 81],
            s->rawCandidatesMutable(),
            81 * sizeof(uint16_t));
    }

    // === CUDA TIMING START ===
    Clock::time_point t0 = Clock::now();

    runCudaLogicalBasic(
        grids.data(),
        cands.data(),
        count);

    Clock::time_point t1 = Clock::now();

    long long cudaMs =
        std::chrono::duration_cast<
        std::chrono::milliseconds>(t1 - t0).count();

    std::cout << "[CUDA] kernel + memcpy: "
        << cudaMs << " ms\n";

    // === WRITEBACK (memcpy) ===
    for (int i = 0; i < count; ++i)
    {
        Sudoku* s = flat[i];

        std::memcpy(
            s->rawGridMutable(),
            &grids[i * 81],
            81);

        std::memcpy(
            s->rawCandidatesMutable(),
            &cands[i * 81],
            81 * sizeof(uint16_t));
    }

    // === CPU FALLBACK ===
    LogicalSolver cpu;
    for (std::vector<Sudoku>& ds : datasets) {
        auto stats = cpu.solveAll(ds);
        std::cout << "[CUDA] CPU fallback: "
			<< stats.alreadySolved << " already solved, "
            << stats.logical << " logical, "
            << stats.backtracking << " backtracking, "
			<< stats.unsolvable << " unsolvable\n";
    }
}
