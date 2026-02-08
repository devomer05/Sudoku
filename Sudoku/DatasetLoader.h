#pragma once
#include <vector>
#include <string>
#include "Sudoku.h"

class DatasetLoader
{
public:
    // dataset/ kök klasörünü alır
    // dataset0..dataset4 -> ayrı ayrı yükler
    static std::vector<std::vector<Sudoku>>
        loadAllDatasets(const std::string& rootFolder, size_t maxSudokuCountToLoad = UINTMAX_MAX);

private:
    // Tek bir datasetX klasörünü yükler
    static std::vector<Sudoku>
        loadSingleDataset(const std::string& datasetFolder, size_t maxSudokuCountToLoad = UINTMAX_MAX);
};