#pragma once
#include <vector>

class Sudoku;

class CUDASolver
{
public:
    CUDASolver();
    ~CUDASolver();

    // Runs CUDA (Naked + Hidden Single) and writes back to Sudoku objects
    void solve(std::vector<std::vector<Sudoku>>& datasets);
};
